// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet::dtls {

bool Dtls::sameTuple(const DtlsPath& a, const DtlsPath& b)
{
    return a.localAddress == b.localAddress && a.localPort == b.localPort && a.peerAddress == b.peerAddress && a.peerPort == b.peerPort;
}

DtlsPath DtlsAssociation::currentPath() const
{
    return {localAddress, localPort, peerAddress, peerPort, localInterfaceId};
}

DtlsPath DtlsAssociation::receivedPath(const Packet *packet) const
{
    auto path = currentPath();
    if (auto address = packet->findTag<L3AddressInd>()) {
        path.localAddress = address->getDestAddress();
        path.peerAddress = address->getSrcAddress();
    }
    if (auto port = packet->findTag<L4PortInd>()) {
        path.localPort = port->getDestPort();
        path.peerPort = port->getSrcPort();
    }
    if (auto interface = packet->findTag<InterfaceInd>())
        path.interfaceId = interface->getInterfaceId();
    return path;
}

const std::vector<uint8_t>& DtlsAssociation::cidForPath(const DtlsPath& path) const
{
    return candidatePath && Dtls::sameTuple(candidatePath->path, path) ? candidatePath->peerCid : peerCid;
}

bool DtlsAssociation::hasMigrationCid() const
{
    if (isDtls12())
        return !peerCid.empty() && std::max(recordOverheadForCid(peerCid), recordOverheadForCid(localCid)) + 27 <= uint64_t(engine.initialPayloadBudget());
    // 22 fixed record bytes, 3 heartbeat header bytes, 8 token bytes and 16 padding bytes.
    return std::any_of(sparePeerCids.begin(), sparePeerCids.end(), [this](const auto& cid) {
        return !cid.empty() && 49 + std::max(cid.size(), localCid.size()) <= uint64_t(engine.initialPayloadBudget());
    });
}

std::vector<uint8_t> DtlsAssociation::takeMigrationCid()
{
    if (isDtls12())
        return peerCid;
    auto it = std::find_if(sparePeerCids.begin(), sparePeerCids.end(), [this](const auto& cid) {
        return !cid.empty() && 49 + std::max(cid.size(), localCid.size()) <= uint64_t(engine.initialPayloadBudget());
    });
    ASSERT(it != sparePeerCids.end());
    auto cid = std::move(*it);
    sparePeerCids.erase(it);
    return cid;
}

bool DtlsAssociation::isNewerRecord(uint64_t epoch, uint64_t number) const
{
    auto record = std::make_pair(epoch, number);
    if (receiveEpoch3 && receiveEpoch3->hasRecords() && record <= std::make_pair(receiveEpoch, receiveEpoch3->getHighestAccepted()))
        return false;
    return !previousReceiveEpoch || !previousReceiveEpoch->hasRecords() ||
           record > std::make_pair(previousReceiveEpochNumber, previousReceiveEpoch->getHighestAccepted());
}

bool DtlsAssociation::considerPath(const DtlsPath& path, bool fresh)
{
    if (Dtls::sameTuple(path, currentPath()))
        return true;
    if (!engine.par("migrationEnabled").boolValue() || state != HandshakeState::ESTABLISHED || closing ||
        !cidNegotiated || localCid.empty() || !heartbeatNegotiated || !peerAllowsHeartbeat ||
        path.localAddress.isUnspecified() || path.peerAddress.isUnspecified() || path.localPort <= 0 || path.peerPort <= 0)
        return false;
    // Old valid records and a competing tuple may deliver, but cannot select or replace a candidate.
    if (fresh && !candidatePath) {
        candidatePath = CandidatePath{path, peerCid, false};
        if (hasMigrationCid())
            candidatePath->peerCid = takeMigrationCid();
        if (probe && probe->purpose == ProbePurpose::PATH_MTU) {
            finishHeartbeatProbe(IDtls::PROBE_CANCELLED);
            return true;
        }
        tryStartPathValidation();
    }
    return true;
}

void DtlsAssociation::tryStartPathValidation()
{
    if (!candidatePath || probe || closing || authenticationActive())
        return;
    auto& owner = engine;
    auto handle = id;
    if (!startHeartbeatProbe(0, ProbePurpose::PATH_VALIDATION)) {
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end() && it->second->candidatePath && !it->second->probe)
            it->second->finishPathValidation(false);
    }
}

void DtlsAssociation::finishPathValidation(bool success)
{
    if (!candidatePath)
        return;
    auto pending = std::move(*candidatePath);
    candidatePath.reset();
    auto& owner = engine;
    auto handle = id;
    if (!success) {
        if (pending.reboundSocket)
            owner.failAssociation(handle, "DTLS path validation failed after local socket rebind");
        return;
    }
    auto maximum = owner.getMaximumDatagramSize(handle);
    auto changedCid = peerCid != pending.peerCid;
    peerCid = std::move(pending.peerCid);
    localAddress = pending.path.localAddress;
    localPort = pending.path.localPort;
    peerAddress = pending.path.peerAddress;
    peerPort = pending.path.peerPort;
    localInterfaceId = pending.path.interfaceId;
    auto path = currentPath();
    if (changedCid)
        owner.emit(cComponent::registerSignal("connectionIdChanged"), (long)handle);
    auto it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    it->second->resetPathMtuDiscovery(maximum);
    it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    owner.emit(cComponent::registerSignal("pathChanged"), (long)handle);
    it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    it->second->callback->pathChanged(handle, path);
    it = owner.associations.find(handle);
    if (it == owner.associations.end() || it->second->closing)
        return;
    it->second->flushAcknowledgements(it->second->sendEpoch);
    it = owner.associations.find(handle);
    if (it == owner.associations.end())
        return;
    if (it->second->flight && !it->second->flight->isComplete() &&
        (!it->second->isDtls12() || it->second->state != HandshakeState::ESTABLISHED)) {
        it->second->flight->requestRetransmission();
        it->second->burstLimit = it->second->burstQueued;
        it->second->pumpFlight();
        it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
    }
    if (it->second->keyUpdate().flight) {
        it->second->keyUpdate().flight->requestRetransmission();
        it->second->pumpUpdate();
        it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
    }
    for (auto& entry : it->second->postHandshakeExchanges)
        if (entry.first != 24 && isOutgoingExchange(entry.first) && entry.second->flight) {
            entry.second->flight->requestRetransmission();
            it->second->pumpPostHandshake(entry.first);
            it = owner.associations.find(handle);
            if (it == owner.associations.end())
                return;
        }
    if (it->second->peerResponseDue && !it->second->keyUpdate().flight && !it->second->isRebinding()) {
        it->second->peerResponseDue = false;
        if (!it->second->requestUpdate(false) && owner.containsAssociation(handle)) {
            owner.failAssociation(handle, "DTLS KeyUpdate response could not resume after path validation");
            return;
        }
    }
    owner.resumeApplications(handle);
}

bool Dtls::migrateLocal(AssociationId id, const L3Address& address, int interfaceId)
{
    Enter_Method("migrateLocal");
    auto it = associations.find(id);
    if (operationalState != OPERATING || !par("migrationEnabled").boolValue() || it == associations.end() ||
        address.isUnspecified() || *par("localAddress").stringValue())
        return false;
    auto& association = *it->second;
    if (association.state != HandshakeState::ESTABLISHED || association.closing || association.candidatePath ||
        (association.probe && association.probe->purpose != Association::ProbePurpose::PATH_MTU) || association.authenticationActive() ||
        !association.cidNegotiated || association.localCid.empty() || association.peerCid.empty() || !association.hasMigrationCid() || nextHeartbeatToken == 0 ||
        association.nextApplicationRecord >= Association::MAX_PROTECTED_RECORDS ||
        !association.heartbeatNegotiated || !association.peerAllowsHeartbeat || !par("heartbeatAcceptRequests").boolValue())
        return false;
    auto table = L3AddressResolver().findInterfaceTableOf(getContainingNode(this));
    if (!table)
        return false;
    auto interface = table->findInterfaceByAddress(address);
    if (!interface || (interfaceId >= 0 && interfaceId != interface->getInterfaceId()) || address.getType() != association.peerAddress.getType())
        return false;
    auto path = association.currentPath();
    path.localAddress = address;
    path.interfaceId = interface->getInterfaceId();
    if (sameTuple(path, association.currentPath()))
        return false;
    auto cid = association.takeMigrationCid();
    association.candidatePath = Association::CandidatePath{path, std::move(cid), false};
    if (association.probe)
        association.finishHeartbeatProbe(IDtls::PROBE_CANCELLED);
    it = associations.find(id);
    if (it == associations.end())
        return false;
    it->second->tryStartPathValidation();
    it = associations.find(id);
    return it != associations.end() && it->second->candidatePath.has_value();
}

bool Dtls::rebindLocalPort(AssociationId id, int port)
{
    Enter_Method("rebindLocalPort");
    auto it = associations.find(id);
    if (operationalState != OPERATING || !par("migrationEnabled").boolValue() || it == associations.end() ||
        associations.size() != 1 || listener || !prevalidationAttempts.empty() || port <= 0 || port > 65535)
        return false;
    auto ready = [this, port](const Association& association) {
        return association.state == HandshakeState::ESTABLISHED && !association.closing && !association.candidatePath && !association.authenticationActive() &&
               association.cidNegotiated && !association.localCid.empty() && !association.peerCid.empty() && association.hasMigrationCid() && nextHeartbeatToken != 0 &&
               association.nextApplicationRecord < Association::MAX_PROTECTED_RECORDS &&
               association.heartbeatNegotiated && association.peerAllowsHeartbeat && par("heartbeatAcceptRequests").boolValue() && port != association.localPort;
    };
    if (!ready(*it->second))
        return false;
    if (it->second->probe)
        it->second->finishHeartbeatProbe(it->second->probe->purpose == Association::ProbePurpose::PATH_MTU ? IDtls::PROBE_CANCELLED : IDtls::PROBE_OUTPUT_REFUSED);
    it = associations.find(id);
    if (it == associations.end() || !ready(*it->second) || it->second->probe)
        return false;
    auto path = it->second->currentPath();
    path.localPort = port;
    auto cid = it->second->takeMigrationCid();
    it->second->candidatePath = Association::CandidatePath{path, std::move(cid), true};
    udpSocket.destroy();
    udpSocket = UdpSocket();
    udpSocket.setOutputGate(gate("socketOut"));
    udpSocket.setCallback(this);
    auto bound = par("localAddress").stringValue();
    udpSocket.bind(*bound ? L3AddressResolver().resolve(bound) : L3Address(), port);
    it->second->tryStartPathValidation();
    return true; // Socket replacement was admitted; subsequent validation failure invalidates the handle.
}

} // namespace inet::dtls
