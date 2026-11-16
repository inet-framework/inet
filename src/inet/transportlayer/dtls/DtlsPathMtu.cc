// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/Dtls12RecordProtection.h"

namespace inet::dtls {

int64_t Dtls::initialPayloadBudget() const
{
    return par(par("pathMtuDiscoveryEnabled").boolValue() ? "pathMtuBase" : "udpPayloadLimit").intValue();
}

int64_t DtlsAssociation::udpPayloadBudget() const
{
    return pathMtu ? int64_t(pathMtu->state.getUdpBudget()) : engine.par("udpPayloadLimit").intValue();
}

void Dtls::sendUdp(Packet *packet, const L3Address& peerAddress, int peerPort)
{
    if (par("pathMtuDiscoveryEnabled").boolValue())
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);
    udpSocket.sendTo(packet, peerAddress, peerPort);
}

size_t Dtls::getMaximumDatagramSize(AssociationId id) const
{
    auto it = associations.find(id);
    if (it == associations.end() || it->second->state != HandshakeState::ESTABLISHED || it->second->closing || it->second->isPathUnavailable())
        return 0;
    auto budget = it->second->udpPayloadBudget() - int64_t(it->second->recordOverhead());
    return budget <= 0 ? 0 : std::min<size_t>(budget, it->second->maximumRecordContent());
}

bool DtlsAssociation::prepareFlight(const std::shared_ptr<DtlsHandshakeFlight>& value)
{
    if (!pathMtu || !value)
        return true;
    auto maximum = std::min<int64_t>(udpPayloadBudget() - int64_t(12 + recordOverhead()), maximumRecordContent() - 12);
    if (maximum <= 0) {
        engine.failAssociation(id, "DTLS path budget cannot carry the negotiated handshake record");
        return false;
    }
    if (value->refragment(maximum)) {
        auto limit = engine.par("maxPendingBytes").intValue();
        if (limit <= 0 || retainedPostHandshakeBytes() + pendingBytes > size_t(limit)) {
            engine.failAssociation(id, "DTLS re-fragmented recovery exceeds the retained-byte bound");
            return false;
        }
    }
    return true;
}

void DtlsAssociation::notifyPathMtu(size_t previousMaximum, bool confirmed)
{
    auto& owner = engine;
    auto handle = id;
    auto maximum = owner.getMaximumDatagramSize(handle);
    if (!confirmed && maximum == previousMaximum)
        return;
    if (!prepareFlight(flight))
        return;
    for (const auto& entry : postHandshakeExchanges)
        if (!prepareFlight(entry.second->flight))
            return;
    if (pathMtu)
        owner.emit(cComponent::registerSignal("confirmedUdpBudget"), maximum ? long(udpPayloadBudget()) : 0L);
    auto it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    it->second->callback->maximumDatagramSizeChanged(handle, maximum);
    it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    // Control recovery remains usable at the base, including while applications are suspended.
    it->second->pumpFlight();
    it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->resumeDeferredOutput();
}

void DtlsAssociation::startPathMtuDiscovery()
{
    if (!pathMtu || closing)
        return;
    if (!heartbeatNegotiated || !peerAllowsHeartbeat) {
        pathMtu->state.unsupported();
        engine.emit(cComponent::registerSignal("probeResult"), long(IDtls::PROBE_UNSUPPORTED));
        return;
    }
    pathMtu->probeDue = true;
    tryStartPathMtuProbe();
}

void DtlsAssociation::resetPathMtuDiscovery(size_t previousMaximum)
{
    if (!pathMtu) {
        notifyPathMtu(previousMaximum);
        return;
    }
    engine.cancelEvent(pathMtu->timer.get());
    pathMtu->raiseAt.reset();
    pathMtu->pausedTimerRemaining.reset();
    pathMtu->state.reset();
    pathMtu->probeDue = true;
    notifyPathMtu(previousMaximum);
}

void DtlsAssociation::tryStartPathMtuProbe()
{
    if (!pathMtu || !pathMtu->probeDue || state != HandshakeState::ESTABLISHED || closing || probe || candidatePath ||
        authenticationActive() || keyUpdate().flight || peerResponseDue)
        return;
    auto size = pathMtu->state.getProbeSize();
    if (size == 0) {
        pathMtu->probeDue = false;
        return;
    }
    if (size < 49 + std::max(localCid.size(), peerCid.size())) {
        engine.failAssociation(id, "DTLS path MTU base cannot carry the negotiated probe profile");
        return;
    }
    if (nextApplicationRecord >= MAX_PROTECTED_RECORDS || engine.nextHeartbeatToken == 0) {
        engine.failAssociation(id, "DTLS path MTU probe record limit reached");
        return;
    }
    pathMtu->probeDue = false;
    engine.cancelEvent(pathMtu->timer.get());
    auto& owner = engine;
    auto handle = id;
    // PATH_MTU returns admitted once it creates the slot, including a reported local output refusal.
    if (!startHeartbeatProbe(size, ProbePurpose::PATH_MTU)) {
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end()) {
            it->second->finishPathMtuProbe(IDtls::PROBE_OUTPUT_REFUSED, size, {});
            owner.emit(cComponent::registerSignal("probeResult"), long(IDtls::PROBE_OUTPUT_REFUSED));
        }
    }
}

void DtlsAssociation::schedulePathMtuProbe()
{
    using Phase = DtlsPathMtuState::Phase;
    if (!pathMtu || closing || pathMtu->state.getPhase() == Phase::UNSUPPORTED)
        return;
    if (pathMtu->pausedAt || authenticationActive()) {
        auto remaining = SimTime(engine.par("pathMtuConfirmationInterval").doubleValue());
        if (pathMtu->state.getPhase() == Phase::SEARCH_COMPLETE && pathMtu->raiseAt)
            remaining = std::min(remaining, std::max(SimTime::fromRaw(1), *pathMtu->raiseAt - pathMtu->pausedAt.value_or(simTime())));
        if (pathMtu->pausedTimerRemaining)
            remaining = std::min(remaining, *pathMtu->pausedTimerRemaining);
        pathMtu->pausedTimerRemaining = remaining;
        return;
    }
    engine.cancelEvent(pathMtu->timer.get());
    auto next = simTime() + SimTime(engine.par("pathMtuConfirmationInterval").doubleValue());
    if (pathMtu->state.getPhase() == Phase::SEARCH_COMPLETE) {
        if (!pathMtu->raiseAt)
            pathMtu->raiseAt = simTime() + SimTime(engine.par("pathMtuRaiseInterval").doubleValue());
        if (*pathMtu->raiseAt <= simTime()) {
            pathMtu->state.raise();
            pathMtu->raiseAt = simTime() + SimTime(engine.par("pathMtuRaiseInterval").doubleValue());
            if (pathMtu->state.getPhase() == Phase::SEARCHING) {
                pathMtu->probeDue = true;
                return;
            }
        }
        next = std::min(next, *pathMtu->raiseAt);
    }
    engine.scheduleAt(next, pathMtu->timer.get());
}

void DtlsAssociation::handlePathMtuTimer()
{
    if (!pathMtu || closing)
        return;
    if (pathMtu->state.getPhase() == DtlsPathMtuState::Phase::SEARCH_COMPLETE && pathMtu->raiseAt && *pathMtu->raiseAt <= simTime()) {
        pathMtu->state.raise();
        pathMtu->raiseAt = simTime() + SimTime(engine.par("pathMtuRaiseInterval").doubleValue());
    }
    pathMtu->probeDue = true;
    tryStartPathMtuProbe();
}

void DtlsAssociation::finishPathMtuProbe(IDtls::ProbeResult result, size_t size, std::optional<size_t> hint)
{
    using Phase = DtlsPathMtuState::Phase;
    if (!pathMtu || closing)
        return;
    if (result == IDtls::PROBE_CANCELLED) {
        pathMtu->probeDue = false;
        schedulePathMtuProbe();
        return;
    }
    if (result == IDtls::PROBE_OUTPUT_REFUSED) {
        pathMtu->probeDue = keyUpdate().flight || peerResponseDue;
        if (!pathMtu->probeDue)
            schedulePathMtuProbe();
        return;
    }
    auto previous = engine.getMaximumDatagramSize(id);
    auto phase = pathMtu->state.getPhase();
    if (result == IDtls::PROBE_SUCCEEDED) {
        if (size != pathMtu->state.getProbeSize())
            return;
        pathMtu->state.succeeded();
    }
    else if (result == IDtls::PROBE_TIMED_OUT)
        pathMtu->state.failed();
    else if (result == IDtls::PROBE_PACKET_TOO_BIG && hint)
        pathMtu->state.packetTooBig(*hint);
    else
        return;
    auto next = pathMtu->state.getPhase();
    pathMtu->probeDue = next == Phase::BASE || next == Phase::SEARCHING;
    if (next != Phase::SEARCH_COMPLETE || phase != Phase::SEARCH_COMPLETE)
        pathMtu->raiseAt.reset();
    auto& owner = engine;
    auto handle = id;
    notifyPathMtu(previous, result == IDtls::PROBE_SUCCEEDED && (phase == Phase::BASE || phase == Phase::ERROR));
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end() && it->second->pathMtu && !it->second->probe && !it->second->pathMtu->probeDue)
        it->second->schedulePathMtuProbe();
}

void DtlsAssociation::handlePacketTooBig(const Packet *quote, int mtu)
{
    if (!pathMtu || !probe || probe->purpose != ProbePurpose::PATH_MTU || !probe->lastHandoff || !quote || mtu <= 0)
        return;
    auto address = quote->findTag<L3AddressInd>();
    auto ports = quote->findTag<L4PortInd>();
    if (!address || !ports || address->getSrcAddress() != probe->path.localAddress || address->getDestAddress() != probe->path.peerAddress ||
        ports->getSrcPort() != probe->path.localPort || ports->getDestPort() != probe->path.peerPort)
        return;
    auto consumed = quote->getFrontOffset().get<B>();
    auto headerBytes = (isDtls12() ? 13 : 5) + probe->lastHandoff->cid.size();
    auto nonceBytes = isDtls12() ? 8 : 0;
    auto innerBytes = isDtls12() && probe->lastHandoff->cid.empty() ? 0 : 1;
    if (consumed < 28 || consumed >= mtu || quote->getDataLength() < B(headerBytes + nonceBytes + 11) || quote->getDataLength() > B(probe->udpBytes))
        return;
    auto hint = size_t(mtu - consumed);
    if (hint >= probe->udpBytes)
        return;
    try {
        std::unique_ptr<Packet> copy(quote->dup());
        const auto& sent = *probe->lastHandoff;
        if (isDtls12()) {
            if (!sent.cid.empty()) {
                auto header = copy->popAtFront<Dtls12CidHeader>(B(headerBytes));
                if (header->getEpoch() != sent.epoch || header->getSequenceNumber() != sent.number ||
                    header->getLength() + headerBytes != probe->udpBytes || header->getConnectionIdArraySize() != sent.cid.size())
                    return;
                for (size_t i = 0; i < sent.cid.size(); ++i)
                    if (header->getConnectionId(i) != sent.cid[i])
                        return;
            }
            else {
                auto header = copy->popAtFront<DtlsPlaintextHeader>(B(13));
                if (header->getContentType() != 24 || header->getLegacyVersion() != DTLS_12 || header->getEpoch() != sent.epoch ||
                    header->getSequenceNumber() != sent.number || header->getLength() + 13 != probe->udpBytes)
                    return;
            }
            if (copy->popAtFront<Dtls12ExplicitNonce>(B(8))->getValue() != Dtls12RecordProtection::explicitNonce(sent.epoch,sent.number))
                return;
        }
        else {
            auto header = copy->popAtFront<DtlsCiphertextHeader>(B(headerBytes));
            if (header->getEpochBits() != (sent.epoch & 3) || header->getSequenceNumberLength() != 2 || !header->getLengthPresent() ||
                header->getSequenceNumber() != (sent.number & 0xffff) || header->getConnectionIdArraySize() != sent.cid.size() ||
                header->getLength() + header->getChunkLength().get<B>() != probe->udpBytes)
                return;
            for (size_t i = 0; i < sent.cid.size(); ++i)
                if (header->getConnectionId(i) != sent.cid[i])
                    return;
        }
        // Only read the quoted native prefix: type, payload length and the eight-byte token.
        // Raw ciphertext cannot supply this modeled correlation metadata and is ignored.
        Ptr<const Chunk> prefix = copy->peekDataAt(b(0), B(11));
        while (auto slice = dynamicPtrCast<const SliceChunk>(prefix)) {
            if (slice->getOffset() != b(0) || slice->getChunkLength() < B(11))
                return;
            prefix = slice->getChunk();
        }
        auto encrypted = dynamicPtrCast<const EncryptedChunk>(prefix);
        auto expectedBody = probe->udpBytes - headerBytes - nonceBytes - DtlsRecordProtection::TAG_BYTES - innerBytes;
        if (!encrypted || !encrypted->getChunk() || encrypted->getChunkLength() != B(expectedBody + innerBytes))
            return;
        Packet retained("quoted DTLS probe metadata", encrypted->getChunk());
        auto heartbeat = retained.peekAtFront<DtlsHeartbeat>(B(expectedBody));
        if (heartbeat->getType() != 1 || heartbeat->getPayloadArraySize() != probe->payload.size())
            return;
        for (size_t i = 0; i < probe->payload.size(); ++i)
            if (heartbeat->getPayload(i) != probe->payload[i])
                return;
    }
    catch (const cRuntimeError&) {
        return;
    }
    finishHeartbeatProbe(IDtls::PROBE_PACKET_TOO_BIG, hint);
}

} // namespace inet::dtls
