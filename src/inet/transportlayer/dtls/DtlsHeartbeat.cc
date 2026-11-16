// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
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

bool DtlsAssociation::startHeartbeatProbe(size_t udpBytes, ProbePurpose purpose)
{
    if (state != HandshakeState::ESTABLISHED || closing || authenticationActive() || !heartbeatNegotiated || !peerAllowsHeartbeat || probe ||
        (purpose != ProbePurpose::PATH_VALIDATION && (keyUpdate().flight || peerResponseDue)) ||
        (purpose == ProbePurpose::PATH_VALIDATION && !candidatePath) || engine.nextHeartbeatToken == 0 ||
        nextApplicationRecord >= (purpose != ProbePurpose::DIAGNOSTIC ? MAX_PROTECTED_RECORDS : uint64_t(engine.par("rekeyAfterRecords").intValue())))
        return false;
    auto path = purpose == ProbePurpose::PATH_VALIDATION ? candidatePath->path : currentPath();
    auto overhead = recordOverheadForCid(cidForPath(path));
    auto minimum = std::max<size_t>(overhead, isDtls12() ? recordOverheadForCid(localCid) : 22 + localCid.size()) + 27;
    if (udpBytes == 0)
        udpBytes = minimum;
    if (udpBytes < minimum || udpBytes > uint64_t(engine.par("udpPayloadLimit").intValue()) || udpBytes - overhead > maximumRecordContent())
        return false;
    probe = std::make_unique<HeartbeatProbe>();
    probe->purpose = purpose;
    if (purpose == ProbePurpose::PATH_MTU)
        probe->retryTimeout = SimTime(engine.par("pathMtuProbeTimeout").doubleValue());
    probe->path = path;
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
    return purpose == ProbePurpose::PATH_MTU || (it != owner.associations.end() && it->second->probe != nullptr);
}

void DtlsAssociation::pumpHeartbeatProbe()
{
    if (!probe || probe->queued || closing || probe->suspended || authenticationActive())
        return;
    auto& owner = engine;
    auto handle = id;
    if (probe->purpose == ProbePurpose::PATH_MTU && probe->transmissions != 0) {
        if (owner.nextHeartbeatToken == 0) {
            finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
            return;
        }
        auto token = owner.nextHeartbeatToken++;
        probe->payload.clear();
        for (int shift = 56; shift >= 0; shift -= 8)
            probe->payload.push_back(token >> shift);
        probe->lastHandoff.reset();
    }
    auto overhead = recordOverheadForCid(cidForPath(probe->path));
    if (probe->udpBytes < overhead + 27) {
        finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
        return;
    }
    auto body = makeHeartbeat(1, probe->payload, probe->udpBytes - overhead - 11);
    probe->queued = true;
    if (!sendHeartbeat(body, true, probe->path)) {
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end() && it->second->probe)
            it->second->finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
    }
}

void DtlsAssociation::finishHeartbeatProbe(IDtls::ProbeResult result, std::optional<size_t> hint)
{
    if (!probe)
        return;
    engine.cancelEvent(probe->retryTimer.get());
    engine.cancelEvent(probe->deadline.get());
    auto purpose = probe->purpose;
    auto size = probe->udpBytes;
    auto& owner = engine;
    auto handle = id;
    probe.reset();
    if (purpose == ProbePurpose::PATH_VALIDATION)
        finishPathValidation(result == IDtls::PROBE_SUCCEEDED);
    else if (purpose == ProbePurpose::PATH_MTU)
        finishPathMtuProbe(result, size, hint);
    owner.emit(cComponent::registerSignal("probeResult"), (long)result);
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->tryStartPathValidation();
    it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->tryStartPathMtuProbe();
}

bool DtlsAssociation::sendHeartbeat(const Ptr<DtlsHeartbeat>& body, bool request, const DtlsPath& path, size_t replyLimit)
{
    auto& owner = engine;
    auto handle = id;
    auto overhead = recordOverheadForCid(cidForPath(path));
    auto bytes = body->getChunkLength().get<B>() + overhead;
    if (closing || bytes > (request ? owner.par("udpPayloadLimit").intValue() : udpPayloadBudget()) || (replyLimit != 0 && bytes > replyLimit))
        return false;
    auto delay = owner.processingDelay(handle, "protectOutDelay");
    auto payload = request && probe ? probe->payload : std::vector<uint8_t>{};
    auto eligible = [&owner, handle, payload, request, overhead, path]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || it->second->closing || it->second->authenticationActive() ||
            it->second->recordOverheadForCid(it->second->cidForPath(path)) != overhead)
            return false;
        auto& association = *it->second;
        if (request)
            return association.probe && association.probe->queued &&
                   (association.probe->purpose == ProbePurpose::PATH_VALIDATION || (!association.keyUpdate().flight && !association.peerResponseDue)) &&
                   association.nextApplicationRecord < (association.probe->purpose != ProbePurpose::DIAGNOSTIC ? MAX_PROTECTED_RECORDS : uint64_t(owner.par("rekeyAfterRecords").intValue())) &&
                   association.probe->payload == payload;
        return true;
    };
    auto skipped = [&owner, handle, request, payload]() {
        auto it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe && it->second->probe->payload == payload) {
            if (it->second->probe->suspended)
                it->second->probe->queued = false;
            else
                it->second->finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
        }
    };
    return owner.enqueueProcessing(owner.outputProcessing.get(), handle, bytes, delay, [&owner, handle, body, request, path, payload]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
        uint64_t number;
        if (!it->second->allocateApplicationRecord(number)) {
            owner.failAssociation(handle, "DTLS heartbeat record limit reached");
            return;
        }
        auto packet = new Packet(request ? "DTLS heartbeat request" : "DTLS heartbeat response", body);
        auto cid = it->second->cidForPath(path);
        auto epoch = it->second->sendEpoch;
        it->second->protectRecord(packet,24,epoch,number,&cid);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!path.localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(path.localAddress);
        if (path.interfaceId >= 0)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(path.interfaceId);
        owner.emit(Dtls::recordOverheadSignal, long(it->second->recordOverheadForCid(cid)));
        owner.emit(Dtls::packetSentToLowerSignal, packet);
        owner.sendUdp(packet, path.peerAddress, path.peerPort);
        it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe && it->second->probe->payload == payload) {
            auto& probe = *it->second->probe;
            probe.queued = false;
            ++probe.transmissions;
            probe.lastHandoff = HeartbeatProbe::Handoff{epoch, number, std::move(cid)};
            if (probe.suspended)
                return;
            if (probe.purpose != ProbePurpose::PATH_MTU && !probe.deadline->isScheduled())
                owner.scheduleAt(simTime() + SimTime(owner.par("probeTimeout").doubleValue()), probe.deadline.get());
            owner.scheduleAt(simTime() + probe.retryTimeout, probe.retryTimer.get());
        }
    }, eligible, skipped);
}

void DtlsAssociation::receiveHeartbeat(uint64_t epoch, uint64_t number, Packet *body, const DtlsPath& path, bool fresh, bool changed, size_t receivedBytes)
{
    // A validation request may cross KeyUpdate on a new path. Previous keys
    // remain usable there; ordinary heartbeat requests still use the current epoch.
    if ((epoch != receiveEpoch && !changed) || !heartbeatNegotiated || authenticationActive() || !engine.hasConsistentEncoding(body))
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
    auto replay = epoch == receiveEpoch ? receiveEpoch3.get() : previousReceiveEpoch.get();
    if (!replay || !replay->accept(number))
        return;
    std::vector<uint8_t> payload;
    for (size_t i = 0; i < heartbeat->getPayloadArraySize(); ++i)
        payload.push_back(heartbeat->getPayload(i));
    if (heartbeat->getType() == 1) {
        if (engine.par("heartbeatAcceptRequests").boolValue())
            sendHeartbeat(makeHeartbeat(2, payload, 16), false, path, changed ? receivedBytes : 0);
    }
    else if (probe && probe->payload == payload && (probe->purpose == ProbePurpose::PATH_MTU ? probe->lastHandoff.has_value() : probe->deadline->isScheduled()) && Dtls::sameTuple(path, probe->path) &&
             (probe->path.interfaceId < 0 || path.interfaceId == probe->path.interfaceId) &&
             (probe->purpose != ProbePurpose::PATH_VALIDATION || fresh)) {
        if (probe->purpose == ProbePurpose::PATH_VALIDATION && candidatePath && candidatePath->path.interfaceId < 0)
            candidatePath->path.interfaceId = path.interfaceId;
        finishHeartbeatProbe(IDtls::PROBE_SUCCEEDED);
    }
}

void DtlsAssociation::suspendHeartbeatForAuthentication()
{
    if (pathMtu && !pathMtu->pausedAt) {
        pathMtu->pausedAt = simTime();
        if (pathMtu->timer->isScheduled())
            pathMtu->pausedTimerRemaining = std::max(SimTime::fromRaw(1), pathMtu->timer->getArrivalTime() - simTime());
        engine.cancelEvent(pathMtu->timer.get());
    }
    if (!probe || probe->suspended)
        return;
    probe->suspended = true;
    if (probe->deadline->isScheduled())
        probe->pausedDeadline = std::max(SimTime::fromRaw(1), probe->deadline->getArrivalTime() - simTime());
    if (probe->purpose == ProbePurpose::PATH_MTU && probe->retryTimer->isScheduled())
        probe->pausedRetry = std::max(SimTime::fromRaw(1), probe->retryTimer->getArrivalTime() - simTime());
    engine.cancelEvent(probe->deadline.get());
    engine.cancelEvent(probe->retryTimer.get());
}

void DtlsAssociation::resumeHeartbeatAfterAuthentication()
{
    if (authenticationActive() || closing)
        return;
    if (pathMtu && pathMtu->pausedAt) {
        if (pathMtu->raiseAt)
            *pathMtu->raiseAt += simTime() - *pathMtu->pausedAt;
        pathMtu->pausedAt.reset();
        if (pathMtu->pausedTimerRemaining) {
            engine.scheduleAt(simTime() + *pathMtu->pausedTimerRemaining, pathMtu->timer.get());
            pathMtu->pausedTimerRemaining.reset();
        }
    }
    if (probe && probe->suspended) {
        probe->suspended = false;
        if (probe->pausedDeadline) {
            engine.scheduleAt(simTime() + *probe->pausedDeadline, probe->deadline.get());
            probe->pausedDeadline.reset();
        }
        if (probe->pausedRetry) {
            engine.scheduleAt(simTime() + *probe->pausedRetry, probe->retryTimer.get());
            probe->pausedRetry.reset();
        }
        else
            pumpHeartbeatProbe();
    }
    else {
        auto& owner = engine;
        auto handle = id;
        tryStartPathValidation();
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end())
            it->second->tryStartPathMtuProbe();
    }
}

} // namespace inet::dtls
