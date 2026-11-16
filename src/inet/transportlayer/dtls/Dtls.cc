// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include <limits>

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

namespace inet::dtls {

Define_Module(Dtls);

const Protocol dtlsProtocol("dtls", "Datagram Transport Layer Security", Protocol::TransportLayer);
simsignal_t Dtls::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t Dtls::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t Dtls::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t Dtls::recordAcceptedSignal = registerSignal("recordAccepted");
simsignal_t Dtls::recordDroppedSignal = registerSignal("recordDropped");
simsignal_t Dtls::associationEstablishedSignal = registerSignal("associationEstablished");
simsignal_t Dtls::associationFailedSignal = registerSignal("associationFailed");
simsignal_t Dtls::establishmentTimeSignal = registerSignal("establishmentTime");
simsignal_t Dtls::recordOverheadSignal = registerSignal("recordOverhead");
simsignal_t Dtls::handshakeRetransmissionSignal = registerSignal("handshakeRetransmission");
simsignal_t Dtls::processingDelaySignal = registerSignal("processingDelay");

Dtls::~Dtls()
{
    discardAll("module destroyed", false);
}

bool Dtls::enqueueProcessing(DtlsProcessingQueue *queue, AssociationId id, size_t bytes, simtime_t delay, DtlsProcessingQueue::Completion completion)
{
    bool accepted = queue && queue->enqueue(id, bytes, delay, std::move(completion));
    if (accepted)
        emit(processingDelaySignal, delay.dbl());
    return accepted;
}

bool Dtls::hasConsistentEncoding(const Packet *packet)
{
    if (packet->getDataLength() == b(0))
        return true;
    try {
        MemoryOutputStream stream;
        Chunk::serialize(stream, packet->peekData(), b(0), packet->getDataLength());
        return stream.getLength() == packet->getDataLength();
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

void Dtls::handleStartOperation(LifecycleOperation *)
{
    auto processingBytes = par("maxProcessingBytes").intValue();
    if (processingBytes <= 0)
        throw cRuntimeError("maxProcessingBytes must be positive");
    inputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsInputProcessing", processingBytes);
    outputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsOutputProcessing", processingBytes);
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
    else if (message->isSelfMessage() && handleHandshakeDeadline(message))
        return;
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
    if (callback == nullptr)
        throw cRuntimeError("DTLS listener callback is null");
    if (listener != nullptr && listener != callback)
        throw cRuntimeError("DTLS listener already registered");
    listener = callback;
}

IDtls::AssociationId Dtls::open(const L3Address& peerAddress, int peerPort, ICallback *callback)
{
    Enter_Method("open");
    if (operationalState != OPERATING)
        return 0;
    if (callback == nullptr || peerAddress.isUnspecified() || peerPort <= 0 || peerPort > 65535)
        throw cRuntimeError("Invalid DTLS open request");
    auto halfOpenLimit = par("maxHalfOpen").intValue();
    auto timeout = SimTime(par("handshakeTimeout").doubleValue());
    auto halfOpen = std::count_if(associations.begin(), associations.end(), [](const auto& entry) { return entry.second->state != HandshakeState::ESTABLISHED; });
    if (halfOpenLimit <= 0 || timeout <= SIMTIME_ZERO)
        throw cRuntimeError("Invalid DTLS handshake limit or timeout");
    if (halfOpen >= halfOpenLimit || nextAssociationId == 0)
        return 0;
    auto association = std::make_unique<Association>(*this);
    association->id = nextAssociationId++;
    association->startedAt = simTime();
    association->callback = callback;
    association->peerAddress = peerAddress;
    association->peerPort = peerPort;
    for (const auto& entry : associations)
        if (entry.second->state == HandshakeState::ESTABLISHED && entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort) {
            association->replacementId = entry.first;
            break;
        }
    association->certificateProfile = std::string(par("profile").stringValue()) == "certificate";
    auto replayWindow = par("replayWindowSize").intValue();
    if (replayWindow <= 0 || replayWindow > 65536)
        throw cRuntimeError("Invalid DTLS replay window size");
    association->receiveEpoch2 = std::make_unique<DtlsRecordState>(replayWindow);
    association->receiveEpoch3 = std::make_unique<DtlsRecordState>(replayWindow);
    association->handshakeDeadline = std::make_unique<cMessage>("dtlsHandshakeDeadline");
    auto id = association->id;
    scheduleAt(simTime() + timeout, association->handshakeDeadline.get());
    associations.emplace(id, std::move(association));
    try {
        sendClientHello(id);
    }
    catch (...) {
        auto it = associations.find(id);
        if (it != associations.end()) {
            cancelEvent(it->second->handshakeDeadline.get());
            associations.erase(it);
        }
        throw;
    }
    return containsAssociation(id) ? id : 0;
}

bool Dtls::handleHandshakeDeadline(cMessage *message)
{
    for (const auto& entry : associations) {
        if (entry.second->handshakeDeadline.get() == message) {
            auto id = entry.first;
            failAssociation(id, "handshake timeout");
            return true;
        }
    }
    return false;
}

void Dtls::sendAlert(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, uint8_t description, bool encrypted, uint64_t sequence)
{
    auto alert = makeShared<DtlsAlert>();
    alert->setLevel(2);
    alert->setDescription(description);
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS fatal alert", alert));
    if (encrypted) {
        DtlsRecordProtection::protect(packet->get(), 21);
        auto header = makeShared<DtlsCiphertextHeader>();
        header->setEpochBits(2);
        header->setSequenceNumber(sequence & 0xffff);
        header->setLength((*packet)->getDataLength().get<B>());
        (*packet)->insertAtFront(header);
    }
    else {
        auto header = makeShared<DtlsPlaintextHeader>();
        header->setContentType(21);
        header->setLength(2);
        (*packet)->insertAtFront(header);
    }
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(localAddress);
    auto delay = encrypted ? SimTime(par("protectOutDelay").doubleValue()) : SIMTIME_ZERO;
    if (!enqueueProcessing(outputProcessing.get(), 0, (*packet)->getByteLength(), delay, [this, peerAddress, peerPort, packet, encrypted]() {
        auto outgoing = packet->release();
        if (encrypted)
            emit(recordOverheadSignal, 22L);
        emit(packetSentToLowerSignal, outgoing);
        udpSocket.sendTo(outgoing, peerAddress, peerPort);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::failAssociation(AssociationId id, const char *reason, int alertDescription)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto association = std::move(it->second);
    associations.erase(it);
    cancelEvent(association->handshakeDeadline.get());
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    if (alertDescription >= 0) {
        bool encrypted = association->state != HandshakeState::CLIENT_WAIT_SERVER_HELLO;
        if (!encrypted || association->nextEpoch2Record < (uint64_t(1) << 24))
            sendAlert(association->localAddress, association->peerAddress, association->peerPort, alertDescription, encrypted, association->nextEpoch2Record++);
    }
    auto callback = association->callback;
    for (auto& packet : association->pendingPackets) {
        auto packetId = packet->getId();
        packet.reset();
        callback->sendError(id, packetId, reason);
    }
    for (auto packetId : association->processingPacketIds)
        callback->sendError(id, packetId, reason);
    emit(associationFailedSignal, (long)id);
    callback->error(id, reason);
}

static void appendUint16(std::vector<uint8_t>& bytes, size_t value)
{
    if (value > 0xffff)
        throw cRuntimeError("TLS vector exceeds its 16-bit length");
    bytes.push_back(value >> 8);
    bytes.push_back(value & 0xff);
}

static void appendExtension(std::vector<uint8_t>& extensions, uint16_t type, const std::vector<uint8_t>& content)
{
    appendUint16(extensions, type);
    appendUint16(extensions, content.size());
    extensions.insert(extensions.end(), content.begin(), content.end());
}

static bool findCookie(const Ptr<const DtlsClientHello>& hello, std::vector<uint8_t>& cookie)
{
    size_t pos = 0;
    size_t end = hello->getExtensionsArraySize();
    while (end - pos >= 4) {
        uint16_t type = (uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1);
        uint16_t length = (uint16_t(hello->getExtensions(pos + 2)) << 8) | hello->getExtensions(pos + 3);
        pos += 4;
        if (length > end - pos)
            return false;
        if (type == 44) {
            if (length < 3 || ((uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1)) != length - 2)
                return false;
            for (size_t i = pos + 2; i < pos + length; ++i)
                cookie.push_back(hello->getExtensions(i));
            return true;
        }
        pos += length;
    }
    return pos == end;
}

static void appendUint64(std::vector<uint8_t>& output, uint64_t value)
{
    for (int i = 7; i >= 0; --i)
        output.push_back(value >> (i * 8));
}

static std::vector<uint8_t> makeCookie(uint64_t issued, const L3Address& local, int localPort,
                                       const L3Address& peer, int peerPort, const Ptr<const DtlsClientHello>& hello)
{
    // Opaque modeled token; it is not a cryptographic authenticator.
    std::vector<uint8_t> token;
    appendUint64(token, issued);
    auto localString = local.str();
    auto peerString = peer.str();
    appendUint16(token, localString.size());
    token.insert(token.end(), localString.begin(), localString.end());
    appendUint16(token, localPort);
    appendUint16(token, peerString.size());
    token.insert(token.end(), peerString.begin(), peerString.end());
    appendUint16(token, peerPort);
    for (size_t i = 0; i < 32; ++i)
        token.push_back(hello->getRandom(i));
    return token;
}

static uint64_t readUint64(const std::vector<uint8_t>& input, size_t offset)
{
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i)
        value = (value << 8) | input[offset + i];
    return value;
}

void Dtls::sendClientHello(AssociationId id, const std::vector<uint8_t>& cookie)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->sendClientHello(cookie);
}
void Dtls::sendRetry(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, const std::vector<uint8_t>& cookie, bool requestKeyShare, size_t receivedBytes)
{
    // RFC 9147 Section 5.1 uses a cookie retry and bounds traffic before address validation.
    auto hello = makeShared<tls::TlsServerHello>();
    hello->setLegacyVersion(0xfefd);
    for (size_t i = 0; i < 32; ++i)
        hello->setRandom(i, Association::HELLO_RETRY_RANDOM[i]);
    std::vector<uint8_t> extensions;
    appendExtension(extensions, 43, {0xfe, 0xfc});
    if (requestKeyShare)
        appendExtension(extensions, 51, {0, 0x1d});
    std::vector<uint8_t> cookieExtension;
    appendUint16(cookieExtension, cookie.size());
    cookieExtension.insert(cookieExtension.end(), cookie.begin(), cookie.end());
    appendExtension(extensions, 44, cookieExtension);
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(40 + extensions.size()));
    auto handshake = makeShared<DtlsHandshakeHeader>();
    handshake->setMessageType(2);
    handshake->setMessageLength(hello->getChunkLength().get<B>());
    handshake->setFragmentLength(handshake->getMessageLength());
    auto record = makeShared<DtlsPlaintextHeader>();
    record->setLength((B(12) + hello->getChunkLength()).get<B>());
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS HelloRetryRequest", hello));
    (*packet)->insertAtFront(handshake);
    (*packet)->insertAtFront(record);
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(localAddress);
    auto bytes = (size_t)(*packet)->getByteLength();
    if (receivedBytes < (bytes + 2) / 3 || bytes > (size_t)par("udpPayloadLimit").intValue()) {
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
        return;
    }
    auto delay = SimTime(par("handshakeOutDelay").doubleValue());
    if (!enqueueProcessing(outputProcessing.get(), 0, bytes, delay, [this, peerAddress, peerPort, packet]() {
        auto outgoing = packet->release();
        emit(packetSentToLowerSignal, outgoing);
        udpSocket.sendTo(outgoing, peerAddress, peerPort);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::sendPlainHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto handshake = makeShared<DtlsHandshakeHeader>();
    handshake->setMessageType(messageType);
    handshake->setMessageSequence(messageSequence);
    handshake->setMessageLength(body->getChunkLength().get<B>());
    handshake->setFragmentLength(handshake->getMessageLength());
    auto record = makeShared<DtlsPlaintextHeader>();
    record->setSequenceNumber(messageSequence);
    auto length = B(12) + body->getChunkLength();
    if (length > B(65535) || length + B(13) > B(par("udpPayloadLimit").intValue()))
        throw cRuntimeError("DTLS handshake message exceeds the configured UDP payload limit");
    record->setLength(length.get<B>());
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS handshake", body));
    (*packet)->insertAtFront(handshake);
    (*packet)->insertAtFront(record);
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!it->second->localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
    auto bytes = (*packet)->getByteLength();
    auto peer = it->second->peerAddress;
    auto port = it->second->peerPort;
    auto delay = SimTime(par("handshakeOutDelay").doubleValue());
    if (!enqueueProcessing(outputProcessing.get(), id, bytes, delay, [this, id, bytes, peer, port, packet]() {
        auto current = associations.find(id);
        if (current != associations.end()) {
            if (current->second->reservedBytes >= (size_t)bytes) {
                current->second->reservedBytes -= bytes;
                current->second->sentBytes += bytes;
            }
            auto outgoing = packet->release();
            emit(packetSentToLowerSignal, outgoing);
            udpSocket.sendTo(outgoing, peer, port);
        }
    }))
        failAssociation(id, "DTLS output processing queue is full");
}

void Dtls::sendProtectedHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body, bool establishAfterSend, bool serverFinished)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto handshake = makeShared<DtlsHandshakeHeader>();
    handshake->setMessageType(messageType);
    handshake->setMessageSequence(messageSequence);
    handshake->setMessageLength(body->getChunkLength().get<B>());
    handshake->setFragmentLength(handshake->getMessageLength());
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS protected handshake", body));
    (*packet)->insertAtFront(handshake);
    DtlsRecordProtection::protect(packet->get(), 22);
    auto record = makeShared<DtlsCiphertextHeader>();
    record->setEpochBits(2);
    if (it->second->nextEpoch2Record >= (uint64_t(1) << 24))
        throw cRuntimeError("DTLS handshake epoch record limit reached");
    record->setSequenceNumber(it->second->nextEpoch2Record++ & 0xffff);
    auto length = (*packet)->getDataLength();
    if (length > B(65535) || length + B(5) > B(par("udpPayloadLimit").intValue()))
        throw cRuntimeError("DTLS protected handshake exceeds the configured UDP payload limit");
    record->setLength(length.get<B>());
    (*packet)->insertAtFront(record);
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!it->second->localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
    auto bytes = (*packet)->getByteLength();
    auto peer = it->second->peerAddress;
    auto port = it->second->peerPort;
    auto handshakeDelay = SimTime(par("handshakeOutDelay").doubleValue());
    auto protectDelay = SimTime(par("protectOutDelay").doubleValue());
    if (!enqueueProcessing(outputProcessing.get(), id, bytes, handshakeDelay, [this, id, bytes, peer, port, protectDelay, packet, establishAfterSend, serverFinished]() {
        if (!enqueueProcessing(outputProcessing.get(), id, bytes, protectDelay, [this, id, bytes, peer, port, packet, establishAfterSend, serverFinished]() {
            auto current = associations.find(id);
            if (current != associations.end()) {
                if (current->second->reservedBytes >= (size_t)bytes) {
                    current->second->reservedBytes -= bytes;
                    current->second->sentBytes += bytes;
                }
                auto outgoing = packet->release();
                emit(recordOverheadSignal, 22L);
                emit(packetSentToLowerSignal, outgoing);
                udpSocket.sendTo(outgoing, peer, port);
                if (establishAfterSend)
                    establish(id);
                else if (serverFinished) {
                    auto current = associations.find(id);
                    if (current != associations.end())
                        current->second->serverFinishedHandedOff();
                }
            }
        }))
            failAssociation(id, "DTLS output processing queue is full");
    }))
        failAssociation(id, "DTLS output processing queue is full");
}

void Dtls::establish(AssociationId id)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto established = std::count_if(associations.begin(), associations.end(), [](const auto& entry) { return entry.second->state == HandshakeState::ESTABLISHED; });
    auto limit = par("maxAssociations").intValue();
    auto old = associations.find(it->second->replacementId);
    bool replacesEstablished = old != associations.end() && old->second->state == HandshakeState::ESTABLISHED;
    if (limit <= 0 || established - (replacesEstablished ? 1 : 0) >= limit) {
        failAssociation(id, "DTLS association limit reached", 40);
        return;
    }
    it->second->state = HandshakeState::ESTABLISHED;
    auto elapsed = simTime() - it->second->startedAt;
    cancelEvent(it->second->handshakeDeadline.get());
    if (replacesEstablished) {
        auto oldId = old->first;
        close(oldId);
        it = associations.find(id);
        if (it == associations.end())
            return;
    }
    auto callback = it->second->callback;
    emit(establishmentTimeSignal, elapsed.dbl());
    emit(associationEstablishedSignal, (long)id);
    callback->established(id);
    while ((it = associations.find(id)) != associations.end() && !it->second->pendingPackets.empty()) {
        auto packet = std::move(it->second->pendingPackets.front());
        it->second->pendingPackets.pop_front();
        it->second->pendingBytes -= std::max(size_t(1), (size_t)packet->getByteLength());
        sendApplication(id, packet.release());
    }
}

void Dtls::sendApplication(AssociationId id, Packet *packet)
{
    // RFC 9147 Sections 4.3 and 4.5.3 bound datagrams and key usage; processing delays are modeled here.
    auto it = associations.find(id);
    if (it == associations.end()) {
        delete packet;
        return;
    }
    auto callback = it->second->callback;
    auto packetId = packet->getId();
    auto bytes = packet->getDataLength();
    if (bytes + B(17) > B(65535) || bytes + B(22) > B(par("udpPayloadLimit").intValue()) || it->second->nextEpoch3Record >= (uint64_t(1) << 24)) {
        delete packet;
        callback->sendError(id, packetId, "DTLS application record size or epoch limit exceeded");
        return;
    }
    auto retained = std::make_shared<std::unique_ptr<Packet>>(packet);
    DtlsRecordProtection::protect(packet, 23);
    auto header = makeShared<DtlsCiphertextHeader>();
    header->setEpochBits(3);
    header->setSequenceNumber(it->second->nextEpoch3Record++ & 0xffff);
    header->setLength(packet->getDataLength().get<B>());
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!it->second->localAddress.isUnspecified())
        packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
    auto peer = it->second->peerAddress;
    auto port = it->second->peerPort;
    auto delay = SimTime(par("protectOutDelay").doubleValue());
    it->second->processingPacketIds.insert(packetId);
    if (!enqueueProcessing(outputProcessing.get(), id, packet->getByteLength(), delay, [this, id, packetId, peer, port, retained]() {
        auto current = associations.find(id);
        if (current != associations.end()) {
            current->second->processingPacketIds.erase(packetId);
            auto outgoing = retained->release();
            emit(recordOverheadSignal, 22L);
            emit(packetSentSignal, outgoing);
            emit(packetSentToLowerSignal, outgoing);
            udpSocket.sendTo(outgoing, peer, port);
        }
    })) {
        auto current = associations.find(id);
        if (current != associations.end())
            current->second->processingPacketIds.erase(packetId);
        retained.reset();
        callback->sendError(id, packetId, "DTLS output processing queue is full");
    }
}

Ptr<Chunk> Dtls::makeCertificate() const
{
    std::string identity = par("identity").stringValue();
    auto bytes = par("certificateBytes").intValue();
    if (identity.empty() || bytes < (int64_t)identity.size() || bytes > 0xffffff - 5)
        throw cRuntimeError("Invalid modeled DTLS certificate size or identity");
    auto certificate = makeShared<tls::TlsCertificate>();
    certificate->setCertificateListArraySize(bytes + 5);
    certificate->setCertificateList(0, (bytes >> 16) & 0xff);
    certificate->setCertificateList(1, (bytes >> 8) & 0xff);
    certificate->setCertificateList(2, bytes & 0xff);
    for (size_t i = 0; i < identity.size(); ++i)
        certificate->setCertificateList(i + 3, identity[i]);
    certificate->setChunkLength(B(bytes + 9));
    return certificate;
}

Ptr<Chunk> Dtls::makeCertificateVerify() const
{
    auto bytes = par("signatureBytes").intValue();
    if (bytes <= 0 || bytes > 65535)
        throw cRuntimeError("Invalid modeled DTLS signature size");
    auto verify = makeShared<tls::TlsCertificateVerify>();
    verify->setSignatureScheme(tls::TlsHandshakeParameters::ECDSA_P256_SHA256);
    verify->setSignatureArraySize(bytes);
    verify->setChunkLength(B(bytes + 4));
    return verify;
}

int Dtls::checkCertificate(Packet *body) const
{
    Ptr<const tls::TlsCertificate> certificate;
    try {
        certificate = body->popAtFront<tls::TlsCertificate>(body->getDataLength());
    }
    catch (const cRuntimeError&) {
        return 50;
    }
    if (body->getDataLength() != b(0))
        return 50;
    auto size = certificate->getCertificateListArraySize();
    if (certificate->getRequestContextArraySize() != 0)
        return 47;
    if (size < 6)
        return 50;
    auto bytes = (uint32_t(certificate->getCertificateList(0)) << 16) |
                 (uint32_t(certificate->getCertificateList(1)) << 8) | certificate->getCertificateList(2);
    std::string identity = par("identity").stringValue();
    if (bytes + 5 != size)
        return 50;
    if (certificate->getCertificateList(bytes + 3) != 0 || certificate->getCertificateList(bytes + 4) != 0)
        return 47;
    if (identity.empty() || bytes < identity.size())
        return 42;
    for (size_t i = 0; i < bytes; ++i)
        if (certificate->getCertificateList(i + 3) != (i < identity.size() ? uint8_t(identity[i]) : 0))
            return 42;
    return 0;
}

int Dtls::checkCertificateVerify(Packet *body) const
{
    Ptr<const tls::TlsCertificateVerify> verify;
    try {
        verify = body->popAtFront<tls::TlsCertificateVerify>(body->getDataLength());
    }
    catch (const cRuntimeError&) {
        return 50;
    }
    if (body->getDataLength() != b(0) || verify->getSignatureArraySize() == 0)
        return 50;
    return verify->getSignatureScheme() == tls::TlsHandshakeParameters::ECDSA_P256_SHA256 ? 0 : 47;
}

void Dtls::sendServerFlight(AssociationId id)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->sendServerFlight();
}
void Dtls::sendFinalAck(AssociationId id, uint64_t sequence)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto ack = makeShared<DtlsAck>();
    ack->setEpochsArraySize(1);
    ack->setEpochs(0, 2);
    ack->setSequenceNumbersArraySize(1);
    ack->setSequenceNumbers(0, sequence);
    ack->setChunkLength(B(18));
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS ACK", ack));
    DtlsRecordProtection::protect(packet->get(), 26);
    auto header = makeShared<DtlsCiphertextHeader>();
    header->setEpochBits(3);
    if (it->second->nextEpoch3Record >= (uint64_t(1) << 24))
        throw cRuntimeError("DTLS application epoch record limit reached");
    header->setSequenceNumber(it->second->nextEpoch3Record++ & 0xffff);
    header->setLength((*packet)->getDataLength().get<B>());
    (*packet)->insertAtFront(header);
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!it->second->localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
    auto peer = it->second->peerAddress;
    auto port = it->second->peerPort;
    auto delay = SimTime(par("protectOutDelay").doubleValue());
    auto bytes = (*packet)->getByteLength();
    if (!enqueueProcessing(outputProcessing.get(), id, bytes, delay, [this, id, peer, port, packet]() {
        if (associations.find(id) != associations.end()) {
            auto outgoing = packet->release();
            emit(recordOverheadSignal, 22L);
            emit(packetSentToLowerSignal, outgoing);
            udpSocket.sendTo(outgoing, peer, port);
        }
    }))
        failAssociation(id, "DTLS output processing queue is full");
}

void Dtls::handlePlainHandshake(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted)
{
    using Parameters = tls::TlsHandshakeParameters;
    if (body->getDataLength() < B(12) || peerAddress.isUnspecified() || peerPort < 0)
        return;
    auto handshake = body->popAtFront<DtlsHandshakeHeader>(B(12));
    if (handshake->getFragmentOffset() != 0 || handshake->getFragmentLength() != handshake->getMessageLength() ||
        body->getDataLength() != B(handshake->getMessageLength()))
        return;
    if (handshake->getMessageType() == 1 && listener && handshake->getMessageSequence() <= 1) {
        auto profile = std::string(par("profile").stringValue());
        if (profile != "psk" && profile != "certificate")
            return;
        bool certificateProfile = profile == "certificate";
        AssociationId replacementId = 0;
        for (const auto& entry : associations)
            if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                entry.second->localAddress == localAddress && entry.second->localPort == localPort) {
                if (entry.second->state == HandshakeState::ESTABLISHED) {
                    replacementId = entry.first;
                    continue;
                }
                if ((entry.second->state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED ||
                     entry.second->state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE) && !entry.second->serverFlightQueued) {
                    if (!*creditCounted) {
                        entry.second->receivedBytes = datagramBytes > std::numeric_limits<size_t>::max() - entry.second->receivedBytes ?
                            std::numeric_limits<size_t>::max() : entry.second->receivedBytes + datagramBytes;
                        *creditCounted = true;
                    }
                    sendServerFlight(entry.first);
                }
                return;
            }
        if (!hasConsistentEncoding(body)) {
            sendAlert(localAddress, peerAddress, peerPort, 50, false);
            return;
        }
        Ptr<const DtlsClientHello> hello;
        try {
            hello = body->popAtFront<DtlsClientHello>(body->getDataLength());
        }
        catch (const cRuntimeError&) {
            sendAlert(localAddress, peerAddress, peerPort, 50, false);
            return;
        }
        if (body->getDataLength() != b(0)) {
            sendAlert(localAddress, peerAddress, peerPort, 50, false);
            return;
        }
        Parameters::Offer offer;
        Parameters::Selection selection;
        auto identity = std::string(par("identity").stringValue());
        auto alert = Association::validateClientHello(hello, certificateProfile, identity, offer, selection);
        if (alert != 0) {
            sendAlert(localAddress, peerAddress, peerPort, alert, false);
            return;
        }
        std::vector<uint8_t> cookie;
        if (!findCookie(hello, cookie)) {
            sendAlert(localAddress, peerAddress, peerPort, 50, false);
            return;
        }
        auto startedAt = simTime();
        if (handshake->getMessageSequence() == 0) {
            if (!cookie.empty())
                return;
            if (par("cookieRequired").boolValue() || selection.retryKeyShare || replacementId != 0) {
                if (*creditCounted)
                    return;
                *creditCounted = true;
                auto issued = uint64_t(simTime().raw());
                auto token = makeCookie(issued, localAddress, localPort, peerAddress, peerPort, hello);
                sendRetry(localAddress, peerAddress, peerPort, token, selection.retryKeyShare, datagramBytes);
                return;
            }
        }
        else {
            auto lifetime = SimTime(par("cookieLifetime").doubleValue()).raw();
            if (cookie.empty()) {
                if (*creditCounted)
                    return;
                *creditCounted = true;
                auto issued = uint64_t(simTime().raw());
                auto token = makeCookie(issued, localAddress, localPort, peerAddress, peerPort, hello);
                sendRetry(localAddress, peerAddress, peerPort, token, selection.retryKeyShare, datagramBytes);
                return;
            }
            if (cookie.size() < 8 || lifetime <= 0 || selection.retryKeyShare) {
                sendAlert(localAddress, peerAddress, peerPort, 47, false);
                return;
            }
            auto issued = readUint64(cookie, 0);
            auto now = uint64_t(simTime().raw());
            if (issued > now || now - issued > uint64_t(lifetime) ||
                cookie != makeCookie(issued, localAddress, localPort, peerAddress, peerPort, hello)) {
                sendAlert(localAddress, peerAddress, peerPort, 47, false);
                return;
            }
            startedAt = SimTime::fromRaw(issued);
        }
        auto maxHalfOpen = par("maxHalfOpen").intValue();
        auto replayWindow = par("replayWindowSize").intValue();
        auto timeout = SimTime(par("handshakeTimeout").doubleValue());
        auto halfOpen = std::count_if(associations.begin(), associations.end(), [](const auto& entry) { return entry.second->state != HandshakeState::ESTABLISHED; });
        if (maxHalfOpen <= 0 || halfOpen >= maxHalfOpen || replayWindow <= 0 || replayWindow > 65536 ||
            timeout <= SIMTIME_ZERO || nextAssociationId == 0) {
            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
            return;
        }
        auto association = std::make_unique<Association>(*this);
        association->id = nextAssociationId++;
        association->startedAt = startedAt;
        association->replacementId = replacementId;
        association->callback = listener;
        association->peerAddress = peerAddress;
        association->peerPort = peerPort;
        association->localAddress = localAddress;
        association->localPort = localPort;
        association->certificateProfile = certificateProfile;
        association->clientAuthRequested = certificateProfile && par("clientAuthentication").boolValue();
        association->addressValidated = handshake->getMessageSequence() == 1;
        association->state = association->clientAuthRequested ? HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE : HandshakeState::SERVER_WAIT_CLIENT_FINISHED;
        association->serverMessageBase = handshake->getMessageSequence();
        association->nextPeerMessage = association->serverMessageBase + 1;
        association->receiveEpoch2 = std::make_unique<DtlsRecordState>(replayWindow);
        association->receiveEpoch3 = std::make_unique<DtlsRecordState>(replayWindow);
        association->receivedBytes = *creditCounted ? 0 : datagramBytes;
        *creditCounted = true;
        association->handshakeDeadline = std::make_unique<cMessage>("dtlsHandshakeDeadline");
        auto id = association->id;
        scheduleAt(simTime() + timeout, association->handshakeDeadline.get());
        associations.emplace(id, std::move(association));
        listener->accepted(id, localAddress, localPort, peerAddress, peerPort);
        if (associations.find(id) != associations.end())
            sendServerFlight(id);
    }
    else if (handshake->getMessageType() == 2) {
        for (const auto& entry : associations) {
            auto& association = *entry.second;
            if (association.peerAddress == peerAddress && association.peerPort == peerPort &&
                (association.localAddress.isUnspecified() || association.localAddress == localAddress) &&
                (association.localPort < 0 || association.localPort == localPort) &&
                association.state == HandshakeState::CLIENT_WAIT_SERVER_HELLO) {
                association.processServerHello(handshake->getMessageSequence(), body, localAddress, localPort);
                return;
            }
        }
    }
}

void Dtls::handleProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes)
{
    auto it = associations.find(id);
    if (it == associations.end() || (epoch == 3 && it->second->state != HandshakeState::ESTABLISHED)) {
        emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
        return;
    }
    if (epoch != 2) {
        handleAvailableProtectedHandshake(id, epoch, truncatedSequence, sequenceBits, body, recordBytes);
        return;
    }
    auto retained = std::make_shared<std::unique_ptr<Packet>>(body->dup());
    if (!inputProcessing->enqueue(id, recordBytes, SIMTIME_ZERO, [this, id, epoch, truncatedSequence, sequenceBits, recordBytes, retained]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        auto state = current->second->state;
        bool waitingForShare = state == HandshakeState::CLIENT_WAIT_SERVER_HELLO;
        bool serverAwaitingFlight = (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE ||
                                     state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY ||
                                     state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED) && !current->second->serverFlightQueued;
        if (waitingForShare || serverAwaitingFlight) {
            emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
            return;
        }
        handleAvailableProtectedHandshake(id, epoch, truncatedSequence, sequenceBits, retained->get(), recordBytes);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::handleAvailableProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes)
{
    // RFC 9147 Sections 4.2.2 and 4.5.1 select an epoch and check its replay window.
    auto it = associations.find(id);
    if (it == associations.end() || (epoch == 3 && it->second->state != HandshakeState::ESTABLISHED))
        return;
    auto replay = epoch == 2 ? it->second->receiveEpoch2.get() : it->second->receiveEpoch3.get();
    auto sequence = replay->reconstruct(truncatedSequence, sequenceBits);
    if (!replay->isAcceptable(sequence)) {
        emit(recordDroppedSignal, (long)REPLAY);
        return;
    }
    auto retained = std::make_shared<std::unique_ptr<Packet>>(body->dup());
    auto delay = SimTime(par("protectInDelay").doubleValue());
    if (!enqueueProcessing(inputProcessing.get(), id, recordBytes, delay, [this, id, epoch, sequence, recordBytes, retained]() {
        auto current = associations.find(id);
        if (current == associations.end() || (epoch == 3 && current->second->state != HandshakeState::ESTABLISHED))
            return;
        auto replay = epoch == 2 ? current->second->receiveEpoch2.get() : current->second->receiveEpoch3.get();
        if (!replay->isAcceptable(sequence))
            return;
        uint8_t contentType;
        if (!DtlsRecordProtection::unprotect(retained->get(), contentType)) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        if (contentType == 21 && (*retained)->getDataLength() == B(2)) {
            try {
                auto alert = (*retained)->popAtFront<DtlsAlert>(B(2));
                if (alert->getLevel() == 2) {
                    replay->accept(sequence);
                    failAssociation(id, "peer sent a fatal DTLS alert");
                }
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
            return;
        }
        if (epoch == 3 && contentType == 23) {
            replay->accept(sequence);
            auto callback = current->second->callback;
            auto incoming = retained->release();
            emit(packetReceivedSignal, incoming);
            emit(packetSentToUpperSignal, incoming);
            drop(incoming);
            callback->dataArrived(id, incoming);
            return;
        }
        if (epoch == 3 && contentType == 26) {
            if ((*retained)->getDataLength() < B(2))
                return;
            if (!hasConsistentEncoding(retained->get())) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
            try {
                (*retained)->popAtFront<DtlsAck>((*retained)->getDataLength());
                if ((*retained)->getDataLength() != b(0)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
            replay->accept(sequence);
            return;
        }
        if (epoch != 2 || contentType != 22) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        auto handshakeDelay = SimTime(par("handshakeInDelay").doubleValue());
        if (!enqueueProcessing(inputProcessing.get(), id, recordBytes, handshakeDelay, [this, id, sequence, retained]() {
            try {
                processProtectedHandshake(id, sequence, retained->get());
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
        }))
            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::processProtectedHandshake(AssociationId id, uint64_t sequence, Packet *body)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->processHandshake(sequence, body);
}
void Dtls::send(AssociationId id, Packet *packet, ICallback *requester)
{
    Enter_Method("send");
    if (packet == nullptr || requester == nullptr)
        throw cRuntimeError("Invalid DTLS send request");
    take(packet);
    if (operationalState != OPERATING) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "DTLS engine is not operating");
        return;
    }
    auto it = associations.find(id);
    if (it == associations.end() || it->second->callback != requester) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "unknown or foreign DTLS association");
        return;
    }
    if (it->second->state == HandshakeState::ESTABLISHED) {
        sendApplication(id, packet);
        return;
    }
    auto bytes = std::max(size_t(1), (size_t)packet->getByteLength());
    auto limit = par("maxPendingBytes").intValue();
    if (limit <= 0 || bytes > (size_t)limit || it->second->pendingBytes > (size_t)limit - bytes) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "DTLS pending application queue is full");
        return;
    }
    it->second->pendingBytes += bytes;
    it->second->pendingPackets.emplace_back(packet);
}

void Dtls::close(AssociationId id)
{
    Enter_Method("close");
    if (operationalState != OPERATING)
        return;
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto association = std::move(it->second);
    auto callback = association->callback;
    cancelEvent(association->handshakeDeadline.get());
    associations.erase(it);
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    for (auto& packet : association->pendingPackets) {
        auto packetId = packet->getId();
        packet.reset();
        callback->sendError(id, packetId, "association closed");
    }
    for (auto packetId : association->processingPacketIds)
        callback->sendError(id, packetId, "association closed");
    callback->closed(id);
}

bool Dtls::requestKeyUpdate(AssociationId, bool)
{
    Enter_Method("requestKeyUpdate");
    return false;
}

void Dtls::discardAll(const char *reason, bool notify)
{
    if (inputProcessing)
        inputProcessing->clear();
    if (outputProcessing)
        outputProcessing->clear();
    for (const auto& entry : associations)
        cancelEvent(entry.second->handshakeDeadline.get());
    auto old = std::move(associations);
    associations.clear();
    if (notify) {
        for (const auto& entry : old) {
            for (auto& packet : entry.second->pendingPackets) {
                auto packetId = packet->getId();
                packet.reset();
                entry.second->callback->sendError(entry.first, packetId, reason);
            }
            for (auto packetId : entry.second->processingPacketIds)
                entry.second->callback->sendError(entry.first, packetId, reason);
            entry.second->callback->error(entry.first, reason);
            entry.second->callback->closed(entry.first);
        }
    }
}

void Dtls::socketDataArrived(UdpSocket *, Packet *packet)
{
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    emit(packetReceivedFromLowerSignal, packet);
    if (packet->hasBitError()) {
        emit(recordDroppedSignal, (long)MALFORMED_RECORD);
        delete packet;
        return;
    }
    parseDatagram(packet);
    delete packet;
}

void Dtls::parseDatagram(Packet *packet)
{
    // RFC 9147 Sections 4.1 and 4.3 permit several DTLS records in one datagram.
    const auto addressTag = packet->findTag<L3AddressInd>();
    const auto portTag = packet->findTag<L4PortInd>();
    L3Address localAddress = addressTag ? addressTag->getDestAddress() : L3Address();
    L3Address peerAddress = addressTag ? addressTag->getSrcAddress() : L3Address();
    int localPort = portTag ? portTag->getDestPort() : -1;
    int peerPort = portTag ? portTag->getSrcPort() : -1;
    size_t datagramBytes = packet->getByteLength();
    auto creditCounted = std::make_shared<bool>(false);
    while (packet->getDataLength() > b(0)) {
        try {
            auto first = packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
            if (first == 21 || first == 22 || first == 26) {
                if (packet->getDataLength() < B(13)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                auto header = packet->removeAtFront<DtlsPlaintextHeader>(B(13));
                // RFC 9147 section 4: legacy_record_version is ignored on receipt.
                if (header->getEpoch() != 0 || packet->getDataLength() < B(header->getLength())) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                emit(recordAcceptedSignal, (long)header->getEpoch());
                if (header->getLength() != 0) {
                    Packet body("DTLS plaintext record", packet->removeAtFront(B(header->getLength())));
                    if (header->getContentType() == 22) {
                        if (body.getDataLength() < B(12)) {
                            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            continue;
                        }
                        auto retained = std::make_shared<std::unique_ptr<Packet>>(body.dup());
                        AssociationId id = 0;
                        auto handshake = body.peekAtFront<DtlsHandshakeHeader>(B(12));
                        if (handshake->getMessageType() != 1)
                            for (const auto& entry : associations)
                                if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                                    (entry.second->localAddress.isUnspecified() || entry.second->localAddress == localAddress) &&
                                    (entry.second->localPort < 0 || entry.second->localPort == localPort) &&
                                    entry.second->state != HandshakeState::ESTABLISHED) {
                                    id = entry.first;
                                    break;
                                }
                        auto delay = SimTime(par("handshakeInDelay").doubleValue());
                        if (!enqueueProcessing(inputProcessing.get(), id, header->getLength() + 13, delay, [this, retained, localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted]() {
                            try {
                                handlePlainHandshake(retained->get(), localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted);
                            }
                            catch (const cRuntimeError&) {
                                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            }
                        }))
                            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
                    }
                    else if (header->getContentType() == 21 && body.getDataLength() == B(2)) {
                        auto alert = body.popAtFront<DtlsAlert>(B(2));
                        if (alert->getLevel() == 2) {
                            AssociationId target = 0;
                            for (const auto& entry : associations)
                                if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                                    (entry.second->localAddress.isUnspecified() || entry.second->localAddress == localAddress) &&
                                    (entry.second->localPort < 0 || entry.second->localPort == localPort) &&
                                    entry.second->state == HandshakeState::CLIENT_WAIT_SERVER_HELLO) {
                                    target = entry.first;
                                    break;
                                }
                            if (target != 0)
                                failAssociation(target, "peer sent a fatal DTLS alert");
                        }
                    }
                }
            }
            else if ((first & 0xe0) == 0x20 && !(first & 0x10)) {
                size_t headerLength = 1 + ((first & 8) ? 2 : 1) + ((first & 4) ? 2 : 0);
                if (packet->getDataLength() < B(headerLength)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                auto header = packet->removeAtFront<DtlsCiphertextHeader>(B(headerLength));
                b length = header->getLengthPresent() ? b(B(header->getLength())) : packet->getDataLength();
                if (length > packet->getDataLength()) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                emit(recordAcceptedSignal, (long)header->getEpochBits());
                auto match = associations.end();
                for (auto it = associations.begin(); it != associations.end(); ++it) {
                    if (it->second->peerAddress == peerAddress && it->second->peerPort == peerPort &&
                        (it->second->localAddress.isUnspecified() || it->second->localAddress == localAddress) &&
                        (it->second->localPort < 0 || it->second->localPort == localPort)) {
                        match = it;
                        if ((header->getEpochBits() == 2 && it->second->state != HandshakeState::ESTABLISHED) ||
                            (header->getEpochBits() == 3 && it->second->state == HandshakeState::ESTABLISHED))
                            break;
                    }
                }
                if (length > b(0)) {
                    auto bodyOffset = packet->getFrontOffset();
                    auto tags = packet->getRegionTags();
                    Packet body("DTLS protected record", packet->removeAtFront(length));
                    body.getRegionTags().copyTags(tags, bodyOffset, body.getFrontOffset(), length);
                    if (match != associations.end() && (header->getEpochBits() == 2 || header->getEpochBits() == 3))
                        handleProtectedHandshake(match->first, header->getEpochBits(), header->getSequenceNumber(), header->getSequenceNumberLength() * 8, &body, (B(headerLength) + length).get<B>());
                    else
                        emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
                }
                else
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
            else {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
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
