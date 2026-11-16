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
    if (state != HandshakeState::ESTABLISHED || closing || !heartbeatNegotiated || !peerAllowsHeartbeat || probe ||
        (purpose == ProbePurpose::DIAGNOSTIC && (keyUpdate().flight || peerResponseDue)) ||
        (purpose == ProbePurpose::PATH_VALIDATION && !candidatePath) || engine.nextHeartbeatToken == 0 ||
        nextApplicationRecord >= (purpose == ProbePurpose::PATH_VALIDATION ? MAX_PROTECTED_RECORDS : uint64_t(engine.par("rekeyAfterRecords").intValue())))
        return false;
    auto path = purpose == ProbePurpose::PATH_VALIDATION ? candidatePath->path : currentPath();
    auto overhead = 22 + (cidNegotiated ? cidForPath(path).size() : 0);
    auto minimum = std::max<size_t>(overhead, 22 + localCid.size()) + 27;
    if (udpBytes == 0)
        udpBytes = minimum;
    if (udpBytes < minimum || udpBytes > uint64_t(engine.par("udpPayloadLimit").intValue()) || udpBytes - overhead > 16384)
        return false;
    probe = std::make_unique<HeartbeatProbe>();
    probe->purpose = purpose;
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
    return it != owner.associations.end() && it->second->probe != nullptr;
}

void DtlsAssociation::pumpHeartbeatProbe()
{
    if (!probe || probe->queued || closing)
        return;
    auto& owner = engine;
    auto handle = id;
    auto overhead = 22 + (cidNegotiated ? cidForPath(probe->path).size() : 0);
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

void DtlsAssociation::finishHeartbeatProbe(IDtls::ProbeResult result)
{
    if (!probe)
        return;
    engine.cancelEvent(probe->retryTimer.get());
    engine.cancelEvent(probe->deadline.get());
    auto purpose = probe->purpose;
    auto& owner = engine;
    auto handle = id;
    probe.reset();
    if (purpose == ProbePurpose::PATH_VALIDATION)
        finishPathValidation(result == IDtls::PROBE_SUCCEEDED);
    owner.emit(cComponent::registerSignal("probeResult"), (long)result);
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->tryStartPathValidation();
}

bool DtlsAssociation::sendHeartbeat(const Ptr<DtlsHeartbeat>& body, bool request, const DtlsPath& path, size_t replyLimit)
{
    auto& owner = engine;
    auto handle = id;
    auto overhead = 22 + (cidNegotiated ? cidForPath(path).size() : 0);
    auto bytes = body->getChunkLength().get<B>() + overhead;
    if (closing || bytes > owner.par("udpPayloadLimit").intValue() || (replyLimit != 0 && bytes > replyLimit))
        return false;
    auto delay = owner.processingDelay(handle, "protectOutDelay");
    auto payload = request && probe ? probe->payload : std::vector<uint8_t>{};
    auto eligible = [&owner, handle, payload, request, overhead, path]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || it->second->closing ||
            22 + (it->second->cidNegotiated ? it->second->cidForPath(path).size() : 0) != overhead)
            return false;
        auto& association = *it->second;
        if (request)
            return association.probe && association.probe->queued &&
                   (association.probe->purpose == ProbePurpose::PATH_VALIDATION || (!association.keyUpdate().flight && !association.peerResponseDue)) &&
                   association.nextApplicationRecord < (association.probe->purpose == ProbePurpose::PATH_VALIDATION ? MAX_PROTECTED_RECORDS : uint64_t(owner.par("rekeyAfterRecords").intValue())) &&
                   association.probe->payload == payload;
        return true;
    };
    auto skipped = [&owner, handle, request, payload]() {
        auto it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe && it->second->probe->payload == payload)
            it->second->finishHeartbeatProbe(IDtls::PROBE_OUTPUT_REFUSED);
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
        DtlsRecordProtection::protect(packet, 24);
        const auto& cid = it->second->cidForPath(path);
        packet->insertAtFront(it->second->makeRecordHeader(it->second->sendEpoch, number, packet->getDataLength().get<B>(), &cid));
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!path.localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(path.localAddress);
        if (path.interfaceId >= 0)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(path.interfaceId);
        owner.emit(Dtls::recordOverheadSignal, long(22 + (it->second->cidNegotiated ? cid.size() : 0)));
        owner.emit(Dtls::packetSentToLowerSignal, packet);
        owner.udpSocket.sendTo(packet, path.peerAddress, path.peerPort);
        it = owner.associations.find(handle);
        if (request && it != owner.associations.end() && !it->second->closing && it->second->probe && it->second->probe->payload == payload) {
            auto& probe = *it->second->probe;
            probe.queued = false;
            if (!probe.deadline->isScheduled())
                owner.scheduleAt(simTime() + SimTime(owner.par("probeTimeout").doubleValue()), probe.deadline.get());
            owner.scheduleAt(simTime() + probe.retryTimeout, probe.retryTimer.get());
        }
    }, eligible, skipped);
}

void DtlsAssociation::receiveHeartbeat(uint64_t epoch, uint64_t number, Packet *body, const DtlsPath& path, bool fresh, bool changed, size_t receivedBytes)
{
    // A validation request may cross KeyUpdate on a new path. Previous keys
    // remain usable there; ordinary heartbeat requests still use the current epoch.
    if ((epoch != receiveEpoch && !changed) || !heartbeatNegotiated || !engine.hasConsistentEncoding(body))
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
    else if (probe && probe->payload == payload && probe->deadline->isScheduled() && Dtls::sameTuple(path, probe->path) &&
             (probe->path.interfaceId < 0 || path.interfaceId == probe->path.interfaceId) &&
             (probe->purpose != ProbePurpose::PATH_VALIDATION || fresh)) {
        if (probe->purpose == ProbePurpose::PATH_VALIDATION && candidatePath && candidatePath->path.interfaceId < 0)
            candidatePath->path.interfaceId = path.interfaceId;
        finishHeartbeatProbe(IDtls::PROBE_SUCCEEDED);
    }
}

} // namespace inet::dtls
