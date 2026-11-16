// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

namespace inet::dtls {

void Dtls::reportEarlyData(AssociationId id, bool accepted)
{
    auto it = associations.find(id);
    if (it == associations.end() || !it->second->earlyRequested || it->second->earlyStatusReported)
        return;
    it->second->earlyStatusReported = true;
    it->second->earlyAccepted = accepted;
    auto callback = it->second->callback;
    callback->earlyDataStatus(id, accepted);
}

void Dtls::sendEarly(AssociationId id, Packet *packet, ICallback *requester)
{
    Enter_Method("sendEarly");
    if (!packet || !requester)
        throw cRuntimeError("Invalid DTLS early send request");
    take(packet);
    auto retained = std::make_shared<std::unique_ptr<Packet>>(packet);
    auto packetId = packet->getId();
    auto it = associations.find(id);
    auto bytes = packet->getDataLength();
    if (operationalState != OPERATING || it == associations.end() || it->second->callback != requester || it->second->serverSide ||
        !it->second->earlyOffered || it->second->retried || it->second->closing || it->second->state == HandshakeState::ESTABLISHED ||
        (it->second->earlyStatusReported && !it->second->earlyAccepted) || bytes.get<b>() % 8 != 0 ||
        bytes > B(DtlsRecordProtection::MAX_CONTENT_BYTES) || bytes + B(it->second->recordOverhead()) > B(par("udpPayloadLimit").intValue()) ||
        uint64_t(bytes.get<B>()) > it->second->offeredSession->earlyDataBytes - it->second->earlySentBytes) {
        retained->reset();
        requester->sendError(id, packetId, "DTLS early send is unavailable or exceeds its allowance");
        return;
    }
    auto size = bytes.get<B>();
    it->second->earlySentBytes += size;
    it->second->processingPacketIds.insert(packetId);
    auto overhead = it->second->recordOverhead();
    auto eligible = [this, id, overhead]() {
        auto current = associations.find(id);
        return current != associations.end() && !current->second->closing && !current->second->retried &&
            current->second->recordOverhead() == overhead &&
            current->second->state != HandshakeState::ESTABLISHED &&
            (!current->second->earlyStatusReported || current->second->earlyAccepted);
    };
    auto skipped = [this, id, packetId, retained, overhead]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return; // teardown already reports tracked application operations
        current->second->processingPacketIds.erase(packetId);
        auto callback = current->second->callback;
        auto changedHeader = current->second->recordOverhead() != overhead;
        retained->reset();
        callback->sendError(id, packetId, changedHeader ? "DTLS early header budget changed before output handoff" : "DTLS early sending ended before output handoff");
    };
    auto delay = processingDelay(id, "protectOutDelay");
    if (!enqueueProcessing(outputProcessing.get(), id, size + overhead, delay, [this, id, packetId, retained]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        if (current->second->nextEarlyRecord >= Association::MAX_PROTECTED_RECORDS) {
            failAssociation(id, "DTLS early record limit exceeded");
            return;
        }
        auto number = current->second->nextEarlyRecord++;
        current->second->processingPacketIds.erase(packetId);
        auto packet = retained->get();
        DtlsRecordProtection::protect(packet, 23);
        auto header = current->second->makeRecordHeader(1, number, packet->getDataLength().get<B>());
        packet->insertAtFront(header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!current->second->localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(current->second->localAddress);
        auto peer = current->second->peerAddress;
        auto port = current->second->peerPort;
        auto outgoing = retained->release();
        emit(recordOverheadSignal, (long)current->second->recordOverhead());
        emit(packetSentSignal, outgoing);
        emit(packetSentToLowerSignal, outgoing);
        udpSocket.sendTo(outgoing, peer, port);
    }, eligible, skipped)) {
        auto current = associations.find(id);
        if (current != associations.end()) {
            current->second->processingPacketIds.erase(packetId);
            current->second->earlySentBytes -= size;
        }
        retained->reset();
        requester->sendError(id, packetId, "DTLS early output processing queue is full");
    }
}

} // namespace inet::dtls
