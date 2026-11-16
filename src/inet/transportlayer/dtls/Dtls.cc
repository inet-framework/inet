// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

Define_Module(Dtls);
const Protocol dtlsProtocol("dtls", "Datagram Transport Layer Security", Protocol::TransportLayer);
simsignal_t Dtls::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t Dtls::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t Dtls::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t Dtls::recordAcceptedSignal = registerSignal("recordAccepted");
simsignal_t Dtls::recordDroppedSignal = registerSignal("recordDropped");

Dtls::~Dtls() { discardAll("module destroyed", false); }

void Dtls::handleStartOperation(LifecycleOperation *)
{
    auto bytes = par("maxProcessingBytes").intValue();
    if (bytes <= 0)
        throw cRuntimeError("maxProcessingBytes must be positive");
    inputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsInputProcessing", bytes);
    outputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsOutputProcessing", bytes);
    udpSocket.setOutputGate(gate("socketOut"));
    udpSocket.setCallback(this);
    const char *address = par("localAddress");
    udpSocket.bind(*address ? L3AddressResolver().resolve(address) : L3Address(), par("localPort"));
}

void Dtls::handleStopOperation(LifecycleOperation *)
{
    listener = nullptr;
    discardAll("node stopped", true);
    udpSocket.close();
    delayActiveOperationFinish(par("closeTimeout"));
}

void Dtls::handleCrashOperation(LifecycleOperation *)
{
    listener = nullptr;
    discardAll("node crashed", true);
    udpSocket.destroy();
}

void Dtls::handleMessageWhenUp(cMessage *message)
{
    if (inputProcessing && inputProcessing->isTimer(message))
        inputProcessing->handleTimer();
    else if (outputProcessing && outputProcessing->isTimer(message))
        outputProcessing->handleTimer();
    else if (udpSocket.belongsToSocket(message))
        udpSocket.processMessage(message);
    else
        throw cRuntimeError("Unexpected message '%s' at DTLS engine", message->getName());
}

void Dtls::listen(ICallback *callback)
{
    Enter_Method("listen");
    if (operationalState != OPERATING)
        return;
    if (!callback || (listener && listener != callback))
        throw cRuntimeError("Invalid DTLS listener registration");
    listener = callback;
}

IDtls::AssociationId Dtls::open(const L3Address& peerAddress, int peerPort, ICallback *callback)
{
    Enter_Method("open");
    if (operationalState != OPERATING)
        return 0;
    if (!callback || peerAddress.isUnspecified() || peerPort <= 0 || peerPort > 65535 || nextAssociationId == 0)
        throw cRuntimeError("Invalid DTLS open request");
    auto association = std::make_unique<Association>();
    association->id = nextAssociationId++;
    association->callback = callback;
    association->peerAddress = peerAddress;
    association->peerPort = peerPort;
    auto id = association->id;
    associations.emplace(id, std::move(association));
    return id;
}

void Dtls::send(AssociationId id, Packet *packet, ICallback *requester)
{
    Enter_Method("send");
    if (!packet || !requester)
        throw cRuntimeError("Invalid DTLS send request");
    take(packet);
    auto packetId = packet->getId();
    delete packet;
    requester->sendError(id, packetId, operationalState == OPERATING ? "association not established" : "DTLS engine is not operating");
}

void Dtls::close(AssociationId id)
{
    Enter_Method("close");
    if (operationalState != OPERATING)
        return;
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto callback = it->second->callback;
    associations.erase(it);
    if (inputProcessing) inputProcessing->cancel(id);
    if (outputProcessing) outputProcessing->cancel(id);
    callback->closed(id);
}

bool Dtls::requestKeyUpdate(AssociationId, bool)
{
    Enter_Method("requestKeyUpdate");
    return false;
}

void Dtls::discardAll(const char *reason, bool notify)
{
    if (inputProcessing) inputProcessing->clear();
    if (outputProcessing) outputProcessing->clear();
    auto old = std::move(associations);
    associations.clear();
    if (notify)
        for (const auto& entry : old) {
            entry.second->callback->error(entry.first, reason);
            entry.second->callback->closed(entry.first);
        }
}

void Dtls::socketDataArrived(UdpSocket *, Packet *packet)
{
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    emit(packetReceivedFromLowerSignal, packet);
    if (packet->hasBitError())
        emit(recordDroppedSignal, (long)MALFORMED_RECORD);
    else
        parseDatagram(packet);
    delete packet;
}

void Dtls::parseDatagram(Packet *packet)
{
    // RFC 9147 Sections 4.1 and 4.3 permit several DTLS records in one datagram.
    while (packet->getDataLength() > b(0)) {
        try {
            auto first = packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
            if (first == 21 || first == 22 || first == 26) {
                if (packet->getDataLength() < B(13))
                    throw cRuntimeError("Truncated DTLS plaintext record");
                auto header = packet->removeAtFront<DtlsPlaintextHeader>(B(13));
                if (header->getEpoch() != 0 || packet->getDataLength() < B(header->getLength()))
                    throw cRuntimeError("Invalid DTLS plaintext record");
                if (header->getLength() != 0)
                    packet->removeAtFront(B(header->getLength()));
                emit(recordAcceptedSignal, (long)header->getEpoch());
            }
            else if ((first & 0xe0) == 0x20 && !(first & 0x10)) {
                size_t headerLength = 1 + ((first & 8) ? 2 : 1) + ((first & 4) ? 2 : 0);
                if (packet->getDataLength() < B(headerLength))
                    throw cRuntimeError("Truncated DTLS unified header");
                auto header = packet->removeAtFront<DtlsCiphertextHeader>(B(headerLength));
                b length = header->getLengthPresent() ? b(B(header->getLength())) : packet->getDataLength();
                if (length > packet->getDataLength())
                    throw cRuntimeError("Invalid DTLS record length");
                emit(recordAcceptedSignal, (long)header->getEpochBits());
                if (length > b(0))
                    packet->removeAtFront(length);
                emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
            }
            else
                throw cRuntimeError("Unknown DTLS record header");
        }
        catch (const cRuntimeError&) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
    }
}

void Dtls::socketErrorArrived(UdpSocket *, Indication *indication)
{
    EV_WARN << "DTLS UDP error " << indication->getName() << endl;
    delete indication;
}

void Dtls::socketClosed(UdpSocket *)
{
    if (operationalState == STOPPING_OPERATION)
        finishActiveOperation();
}

} // namespace inet::dtls
