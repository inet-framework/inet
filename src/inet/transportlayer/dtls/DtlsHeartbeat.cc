// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

namespace inet::dtls {

bool Dtls::startHeartbeatProbe(AssociationId id, size_t udpBytes)
{
    Enter_Method("startHeartbeatProbe");
    auto it = associations.find(id);
    return operationalState == OPERATING && it != associations.end() && it->second->startHeartbeatProbe(udpBytes);
}

Ptr<DtlsHeartbeat> DtlsAssociation::makeHeartbeat(uint8_t type, const std::vector<uint8_t>& payload, size_t padding)
{
    auto body = makeShared<DtlsHeartbeat>();
    body->setType(type);
    body->setPayloadArraySize(payload.size());
    for (size_t i = 0; i < payload.size(); ++i)
        body->setPayload(i, payload[i]);
    body->setPaddingArraySize(padding);
    for (size_t i = 0; i < padding; ++i)
        body->setPadding(i, engine.intuniform(0, 255));
    body->setChunkLength(B(3 + payload.size() + padding));
    body->markImmutable();
    return body;
}

bool DtlsAssociation::startHeartbeatProbe(size_t udpBytes)
{
    if (state != HandshakeState::ESTABLISHED || closing || !heartbeatNegotiated || !peerAllowsHeartbeat || probe ||
        keyUpdate().flight || peerResponseDue || engine.nextHeartbeatToken == 0 ||
        nextApplicationRecord >= uint64_t(engine.par("rekeyAfterRecords").intValue()))
        return false;
    auto minimum = std::max<size_t>(recordOverhead(), 22 + localCid.size()) + 27;
    if (udpBytes == 0)
        udpBytes = minimum;
    if (udpBytes < minimum || udpBytes > uint64_t(engine.par("udpPayloadLimit").intValue()) || udpBytes - recordOverhead() > 16384)
        return false;
    probe = std::make_unique<HeartbeatProbe>();
    auto token = engine.nextHeartbeatToken++;
    for (int shift = 56; shift >= 0; shift -= 8)
        probe->payload.push_back(token >> shift);
    probe->udpBytes = udpBytes;
    probe->retryTimer = std::make_unique<cMessage>("dtlsProbeRetry");
    probe->deadline = std::make_unique<cMessage>("dtlsProbeDeadline");
    auto& owner = engine;
    auto handle = id;
    pumpHeartbeatProbe();
    auto it = owner.associations.find(handle);
    return it != owner.associations.end() && it->second->probe != nullptr;
}

void DtlsAssociation::pumpHeartbeatProbe()
{
    if (!probe || probe->queued || closing)
        return;
    auto& owner = engine;
    auto handle = id;
    if (probe->udpBytes < recordOverhead() + 27) {
        finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
        return;
    }
    auto body = makeHeartbeat(1, probe->payload, probe->udpBytes - recordOverhead() - 11);
    probe->queued = true;
    if (!sendHeartbeat(body, true)) {
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end() && it->second->probe)
            it->second->finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
    }
}

void DtlsAssociation::finishHeartbeatProbe(IDtls::ProbeResult result)
{
    if (!probe)
        return;
    engine.cancelEvent(probe->retryTimer.get());
    engine.cancelEvent(probe->deadline.get());
    probe.reset();
    engine.emit(cComponent::registerSignal("probeResult"), (long)result);
}

bool DtlsAssociation::sendHeartbeat(const Ptr<DtlsHeartbeat>& body, bool request)
{
    auto& owner = engine;
    auto handle = id;
    auto overhead = recordOverhead();
    auto bytes = body->getChunkLength().get<B>() + overhead;
    if (closing || bytes > owner.par("udpPayloadLimit").intValue())
        return false;
    auto delay = owner.processingDelay(handle, "protectOutDelay");
    auto payload = request && probe ? probe->payload : std::vector<uint8_t>{};
    auto eligible = [&owner, handle, payload, request, overhead]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || it->second->closing || it->second->recordOverhead() != overhead)
            return false;
        auto& association = *it->second;
        if (request)
            return association.probe && association.probe->queued && !association.keyUpdate().flight && !association.peerResponseDue &&
                   association.nextApplicationRecord < uint64_t(owner.par("rekeyAfterRecords").intValue()) &&
                   association.probe->payload == payload;
        return true;
    };
    auto skipped = [&owner, handle, request, payload]() {
        auto it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe && it->second->probe->payload == payload)
            it->second->finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
    };
    return owner.enqueueProcessing(owner.outputProcessing.get(), handle, bytes, delay, [&owner, handle, body, request]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
        uint64_t number;
        if (!it->second->allocateApplicationRecord(number)) {
            owner.failAssociation(handle, "DTLS heartbeat record limit reached");
            return;
        }
        auto packet = new Packet(request ? "DTLS heartbeat request" : "DTLS heartbeat response", body);
        DtlsRecordProtection::protect(packet, 24);
        packet->insertAtFront(it->second->makeRecordHeader(it->second->sendEpoch, number, packet->getDataLength().get<B>()));
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!it->second->localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
        auto peer = it->second->peerAddress;
        auto port = it->second->peerPort;
        owner.emit(Dtls::recordOverheadSignal, (long)it->second->recordOverhead());
        owner.emit(Dtls::packetSentToLowerSignal, packet);
        owner.udpSocket.sendTo(packet, peer, port);
        it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe) {
            auto& probe = *it->second->probe;
            probe.queued = false;
            if (!probe.deadline->isScheduled())
                owner.scheduleAt(simTime() + SimTime(owner.par("probeTimeout").doubleValue()), probe.deadline.get());
            owner.scheduleAt(simTime() + probe.retryTimeout, probe.retryTimer.get());
        }
    }, eligible, skipped);
}

void DtlsAssociation::receiveHeartbeat(uint64_t epoch, uint64_t number, Packet *body)
{
    if (epoch != receiveEpoch || !heartbeatNegotiated || !engine.hasConsistentEncoding(body))
        return;
    Ptr<const DtlsHeartbeat> heartbeat;
    try {
        heartbeat = body->popAtFront<DtlsHeartbeat>(body->getDataLength());
        if (body->getDataLength() != b(0) || (heartbeat->getType() != 1 && heartbeat->getType() != 2))
            return;
    }
    catch (const cRuntimeError&) {
        return;
    }
    if (!receiveEpoch3->accept(number))
        return;
    std::vector<uint8_t> payload;
    for (size_t i = 0; i < heartbeat->getPayloadArraySize(); ++i)
        payload.push_back(heartbeat->getPayload(i));
    if (heartbeat->getType() == 1) {
        if (engine.par("heartbeatAcceptRequests").boolValue())
            sendHeartbeat(makeHeartbeat(2, payload, 16), false);
    }
    else if (probe && probe->payload == payload && probe->deadline->isScheduled())
        finishHeartbeatProbe(IDtls::PROBE_SUCCEEDED);
}

} // namespace inet::dtls
