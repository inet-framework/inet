// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/hwmp/Hwmp.h"

#include <algorithm>
#include <tuple>
#include <utility>
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mesh/MeshDataAttempt.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {
namespace ieee80211 {

Define_Module(Hwmp);
simsignal_t Hwmp::pathUpdatedSignal = registerSignal("hwmpPathUpdated");
simsignal_t Hwmp::discoveryChangedSignal = registerSignal("hwmpDiscoveryChanged");
simsignal_t Hwmp::rootUpdatedSignal = registerSignal("hwmpRootUpdated");

void Hwmp::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this,"mibModule",true);
        management.reference(this,"managementModule",true);
        linkMetric.reference(this,"metricModule",true);
        forwarding.reference(this,"forwardingModule",true);
        hcf.reference(this,"hcfModule",true);
        preqMinInterval = par("preqMinInterval");
        perrMinInterval = par("perrMinInterval");
        traversalTime = par("netDiameterTraversalTime");
        activePathTimeout = par("activePathTimeout");
        maxRetries = par("maxPreqRetries");
        initialTtl = par("initialTtl");
        seenCapacity = par("seenIdentityCapacity");
        int configuredRootMode = par("rootMode");
        if (configuredRootMode != NO_ROOT && configuredRootMode != PREQ_NO_PREP && configuredRootMode != PREQ_WITH_PREP && configuredRootMode != RANN)
            throw cRuntimeError("Unsupported HWMP root mode");
        rootMode = static_cast<RootMode>(configuredRootMode);
        rootInterval = par("rootInterval"); rannInterval = par("rannInterval");
        confirmationInterval = par("confirmationInterval"); activePathToRootTimeout = par("activePathToRootTimeout");
        auto integralTus = [](simtime_t value, uint32_t maximum) {
            return value > SIMTIME_ZERO && value.raw() % SimTime(1024,SIMTIME_US).raw() == 0 && value.inUnit(SIMTIME_US)/1024 <= maximum;
        };
        if (!integralTus(rootInterval,65535) || !integralTus(rannInterval,65535) || !integralTus(confirmationInterval,65535)
                || !integralTus(activePathToRootTimeout,UINT32_MAX))
            throw cRuntimeError("HWMP root intervals require integral TUs in1..65535; root path lifetime requires integral TUs in1..UINT32_MAX");
        pathExpiryTimer = new cMessage("hwmpPathExpiry");
        perrTimer = new cMessage("hwmpPerr");
        rootWorkTimer = new cMessage("hwmpRootWork");
        if (rootMode != NO_ROOT) rootOriginTimer = new cMessage("hwmpRootOrigin");
        if (preqMinInterval <= SIMTIME_ZERO || perrMinInterval <= SIMTIME_ZERO || traversalTime <= SIMTIME_ZERO || activePathTimeout <= SIMTIME_ZERO
                || maxRetries < 0 || initialTtl < 1 || initialTtl > 255 || seenCapacity < 1
                || activePathTimeout.raw() % SimTime(1024,SIMTIME_US).raw() != 0
                || activePathTimeout.inUnit(SIMTIME_US)/1024 > UINT32_MAX)
            throw cRuntimeError("Invalid HWMP timing, TTL or history policy");
        WATCH(ownSequence);
        WATCH(nextDiscoveryId);
        WATCH_EXPR("retainedPaths",paths.size());
        WATCH_MAP(paths);
        WATCH_MAP(selfPrecursors);
        WATCH_EXPR("discoveryRecords",describeDiscoveries());
        WATCH_EXPR("seenIdentities",getSeenIdentityCount());
        WATCH_EXPR("pendingPathErrors",pendingErrors.size());
        WATCH_EXPR("lastPerrHandoff",lastPerrHandoff ? lastPerrHandoff->str() : "none");
        WATCH(rannInterval);
        WATCH_EXPR("rootContexts",roots.size());
        WATCH_MAP(roots);
        WATCH_EXPR("rootWorkDecisions",describeRootWork());
        WATCH_EXPR("lastRootPreqHandoff",lastRootPreq ? lastRootPreq->str() : "none");
        WATCH_EXPR("lastOwnRannHandoff",lastOwnRann ? lastOwnRann->str() : "none");
        WATCH_EXPR("lastRannActionHandoff",lastRannAction ? lastRannAction->str() : "none");
        WATCH_EXPR("lastConfirmationActionHandoff",lastConfirmationAction ? lastConfirmationAction->str() : "none");
    }
}

Hwmp::~Hwmp()
{
    unsubscribeFailureNotifications();
    cancelAndDelete(pathExpiryTimer);
    cancelAndDelete(perrTimer);
    cancelAndDelete(rootOriginTimer);
    cancelAndDelete(rootWorkTimer);
    for (auto& entry : discoveries)
        cancelAndDelete(entry.second.timer);
}

void Hwmp::handleStartOperation(LifecycleOperation *)
{
    paths.clear(); selfPrecursors.clear(); seen.clear();
    mib->subscribe(Ieee80211Mib::meshPeerChangedSignal,this);
    hcf->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
    hcf->subscribe(Hcf::meshTerminalRecoverySignal,this);
    if (rootMode != NO_ROOT)
        scheduleAfter(rootMode == RANN ? rannInterval : rootInterval,rootOriginTimer);
}

void Hwmp::unsubscribeFailureNotifications()
{
    if (mib && mib->isSubscribed(Ieee80211Mib::meshPeerChangedSignal,this))
        mib->unsubscribe(Ieee80211Mib::meshPeerChangedSignal,this);
    if (hcf && hcf->isSubscribed(Hcf::meshDataAttemptCompletedSignal,this))
        hcf->unsubscribe(Hcf::meshDataAttemptCompletedSignal,this);
    if (hcf && hcf->isSubscribed(Hcf::meshTerminalRecoverySignal,this))
        hcf->unsubscribe(Hcf::meshTerminalRecoverySignal,this);
}

void Hwmp::receiveSignal(cComponent *, simsignal_t signal, bool value, cObject *details)
{
    Enter_Method_Silent();
    if (operationalState == OPERATING && signal == Ieee80211Mib::meshPeerChangedSignal) {
        if (!value) invalidatePeer(check_and_cast<Ieee80211Mib::MeshPeerChange *>(details)->peer,true);
        else processRootWork();
    }
}

void Hwmp::receiveSignal(cComponent *, simsignal_t signal, cObject *, cObject *details)
{
    Enter_Method_Silent();
    if (operationalState == OPERATING && signal == Hcf::meshDataAttemptCompletedSignal) {
        auto attempt = check_and_cast<MeshDataAttempt *>(details);
        if (!attempt->acknowledged && attempt->terminalTimeout && mib->isMeshPeerEstablished(attempt->peer))
            invalidatePeer(attempt->peer,false);
    }
    else if (operationalState == OPERATING && signal == Hcf::meshTerminalRecoverySignal) {
        auto recovery = check_and_cast<MeshTerminalRecovery *>(details);
        if (mib->isMeshPeerEstablished(recovery->peer))
            invalidatePeer(recovery->peer,false);
    }
}

void Hwmp::invalidatePeer(MacAddress peer, bool closed)
{
    std::map<MacAddress,std::optional<PathInfo>> before;
    auto priorSelf = selfPrecursors;
    std::vector<PendingError> errors;
    for (auto& entry : paths) {
        auto& path = entry.second;
        before.emplace(entry.first,path);
        if (path.validated && path.sequence && simTime() < path.expiry && path.nextHop == peer) {
            ++*path.sequence;
            path.validated = false;
            HwmpPerrDestination destination;
            destination.address = entry.first; destination.sequenceNumber = *path.sequence;
            destination.reasonCode = RC_MESH_PATH_ERROR_DESTINATION_UNREACHABLE;
            for (const auto& precursor : path.precursors)
                if (simTime() < precursor.second && mib->isMeshPeerEstablished(precursor.first) && precursor.first != peer)
                    errors.push_back({precursor.first,uint8_t(initialTtl),destination,precursor.second});
        }
        if (closed) path.precursors.erase(peer);
    }
    if (closed) {
        selfPrecursors.erase(peer);
        pendingErrors.erase(std::remove_if(pendingErrors.begin(),pendingErrors.end(),[&](const PendingError& error) { return error.recipient == peer; }),pendingErrors.end());
        before.emplace(mib->address,std::nullopt);
    }
    schedulePathExpiry();
    publishPathChanges(before,priorSelf);
    queueErrors(errors);
    processRootWork();
}

void Hwmp::stopDiscoveries()
{
    cancelEvent(pathExpiryTimer);
    std::vector<MacAddress> failed;
    std::vector<MacAddress> removed;
    for (const auto& entry : paths) removed.push_back(entry.first);
    if (!selfPrecursors.empty()) removed.push_back(mib->address);
    for (auto& entry : discoveries) {
        cancelEvent(entry.second.timer);
        if (entry.second.attempts) failed.push_back(entry.first);
        entry.second.attempts.reset();
        entry.second.sourceDue.reset();
    }
    paths.clear(); selfPrecursors.clear(); seen.clear();
    for (auto destination : removed) publishPath(destination);
    for (auto destination : failed) publishDiscovery(destination);
    for (auto destination : failed)
        forwarding->discoveryFailed(destination);
}
void Hwmp::handleStopOperation(LifecycleOperation *)
{
    auto before = observeRootState();
    unsubscribeFailureNotifications(); clearRootWork(); cancelEvent(perrTimer); pendingErrors.clear(); stopDiscoveries();
    publishRootChanges(before);
}
void Hwmp::handleCrashOperation(LifecycleOperation *)
{
    auto before = observeRootState();
    unsubscribeFailureNotifications(); clearRootWork(); cancelEvent(perrTimer); pendingErrors.clear(); stopDiscoveries();
    publishRootChanges(before);
}

std::optional<MeshPath> Hwmp::findPath(MacAddress destination) const
{
    if (operationalState != OPERATING || destination == mib->address)
        return std::nullopt;
    auto it = paths.find(destination);
    if (it == paths.end() || !it->second.validated || !it->second.sequence || simTime() >= it->second.expiry
            || !mib->isMeshPeerEstablished(it->second.nextHop))
        return std::nullopt;
    return MeshPath{it->second.nextHop,it->second.expiry};
}

std::optional<Hwmp::PathInfo> Hwmp::getPathInfo(MacAddress destination) const
{
    auto it = paths.find(destination);
    return it == paths.end() ? std::nullopt : std::optional<PathInfo>(it->second);
}

simtime_t Hwmp::getPrecursorExpiry(MacAddress destination, MacAddress precursor) const
{
    if (operationalState != OPERATING || !mib->isMeshPeerEstablished(precursor)) return SIMTIME_ZERO;
    const auto *entries = &selfPrecursors;
    if (destination != mib->address) {
        auto path = paths.find(destination);
        if (path == paths.end()) return SIMTIME_ZERO;
        entries = &path->second.precursors;
    }
    auto it = entries->find(precursor);
    return it == entries->end() || simTime() >= it->second ? SIMTIME_ZERO : it->second;
}

void Hwmp::addPrecursor(MacAddress destination, MacAddress precursor, simtime_t expiry)
{
    auto& entries = destination == mib->address ? selfPrecursors : paths.at(destination).precursors;
    auto& stored = entries[precursor];
    stored = std::max(stored,expiry);
}

void Hwmp::refreshPath(MacAddress destination)
{
    Enter_Method("refreshPath");
    if (operationalState != OPERATING) return;
    auto it = paths.find(destination);
    if (it != paths.end()) {
        auto before = it->second;
        it->second.validated &= simTime() < it->second.expiry && mib->isMeshPeerEstablished(it->second.nextHop);
        it->second.expiry = simTime()+activePathTimeout;
        schedulePathExpiry();
        if (before != it->second) publishPath(destination);
    }
}
void Hwmp::refreshPrecursor(MacAddress destination, MacAddress precursor)
{
    Enter_Method("refreshPrecursor");
    if (operationalState == OPERATING && (destination == mib->address || paths.count(destination))) {
        auto before = getPathInfo(destination);
        auto priorSelf = selfPrecursors;
        addPrecursor(destination,precursor,simTime()+activePathTimeout);
        publishPathChanges({{destination,before}},priorSelf);
    }
}

bool Hwmp::stale(MacAddress destination, uint32_t sequence) const
{
    auto it = paths.find(destination);
    return it != paths.end() && it->second.sequence && sequence != *it->second.sequence && !isNewer(sequence,*it->second.sequence);
}

bool Hwmp::updatePath(MacAddress destination, MacAddress transmitter, uint32_t sequence, uint32_t metric, unsigned int hops, simtime_t expiry)
{
    auto it = paths.find(destination);
    if (it != paths.end() && it->second.sequence && !isNewer(sequence,*it->second.sequence)
            && (sequence != *it->second.sequence || metric >= it->second.metric))
        return false;
    auto& path = paths[destination];
    //14.11.8.4/Table14-9: PREQ does not validate a new reverse path.
    path.validated &= simTime() < path.expiry && mib->isMeshPeerEstablished(path.nextHop);
    path.nextHop = transmitter;
    path.sequence = sequence;
    path.metric = metric;
    path.hops = hops;
    path.expiry = std::max(path.expiry,expiry);
    return true;
}

void Hwmp::expireSeen()
{
    for (auto it = seen.begin(); it != seen.end(); )
        if (simTime() >= it->second) it = seen.erase(it); else ++it;
}
size_t Hwmp::getSeenIdentityCount() const
{
    size_t count = 0;
    for (const auto& entry : seen) count += simTime() < entry.second;
    return count;
}

std::string Hwmp::describeDiscoveries() const
{
    std::ostringstream out;
    for (const auto& entry : discoveries) {
        out << entry.first << " attempts=";
        if (entry.second.attempts) out << *entry.second.attempts; else out << "idle";
        if (entry.second.lastSent) out << " lastPREQ=" << *entry.second.lastSent;
        if (entry.second.sourceDue) out << " sourceDue=" << *entry.second.sourceDue;
        if (entry.second.timer && entry.second.timer->isScheduled()) out << " next=" << entry.second.timer->getArrivalTime();
        out << "; ";
    }
    return out.str();
}

void Hwmp::publishPath(MacAddress destination)
{
    PathChange change(destination,getPathInfo(destination),destination == mib->address ? selfPrecursors : std::map<MacAddress,simtime_t>{});
    emit(pathUpdatedSignal,findPath(destination).has_value(),&change);
}
void Hwmp::publishPathChanges(const std::map<MacAddress,std::optional<PathInfo>>& before, const std::map<MacAddress,simtime_t>& priorSelf)
{
    for (const auto& entry : before) {
        bool changed = entry.first == mib->address ? priorSelf != selfPrecursors : entry.second != getPathInfo(entry.first);
        if (changed) publishPath(entry.first);
    }
}
void Hwmp::publishDiscovery(MacAddress destination)
{
    auto it = discoveries.find(destination);
    DiscoveryChange change(destination,getDiscoveryAttempts(destination),getDiscoveryDeadline(destination),
        it == discoveries.end() ? std::nullopt : it->second.lastSent);
    emit(discoveryChangedSignal,isDiscoveryActive(destination),&change);
}

void Hwmp::schedulePathExpiry()
{
    cancelEvent(pathExpiryTimer);
    if (operationalState != OPERATING) return;
    simtime_t next = SIMTIME_MAX;
    for (const auto& entry : paths)
        if (entry.second.validated && simTime() < entry.second.expiry) next = std::min(next,entry.second.expiry);
    if (next != SIMTIME_MAX) scheduleAt(next,pathExpiryTimer);
}

bool Hwmp::isDiscoveryActive(MacAddress destination) const
{
    auto it = discoveries.find(destination);
    return it != discoveries.end() && it->second.attempts.has_value();
}
int Hwmp::getDiscoveryAttempts(MacAddress destination) const
{
    auto it = discoveries.find(destination);
    return it == discoveries.end() ? 0 : it->second.attempts.value_or(0);
}
simtime_t Hwmp::getDiscoveryDeadline(MacAddress destination) const
{
    auto it = discoveries.find(destination);
    if (it == discoveries.end() || !it->second.attempts || !it->second.sourceDue) return SIMTIME_ZERO;
    return *it->second.sourceDue;
}

void Hwmp::startDiscovery(MacAddress destination)
{
    Enter_Method("startDiscovery");
    if (operationalState != OPERATING || destination.isUnspecified() || destination.isMulticast() || destination == mib->address
            || isDiscoveryActive(destination)) return;
    auto& discovery = discoveries[destination];
    if (!discovery.timer) discovery.timer = new cMessage("hwmpDiscovery");
    discovery.attempts = 0;
    discovery.sourceDue = simTime();
    attemptDiscovery(destination);
    if (isDiscoveryActive(destination) && getDiscoveryAttempts(destination) == 0) publishDiscovery(destination);
}
void Hwmp::cancelDiscovery(MacAddress destination)
{
    Enter_Method("cancelDiscovery");
    auto it = discoveries.find(destination);
    if (it != discoveries.end()) {
        bool wasActive = it->second.attempts.has_value();
        cancelEvent(it->second.timer);
        it->second.attempts.reset(); // retain only minimum-interval history and reusable timer
        it->second.sourceDue.reset();
        if (wasActive) publishDiscovery(destination);
    }
    processRootWork();
}

void Hwmp::attemptDiscovery(MacAddress)
{
    processRootWork();
}

bool Hwmp::sourceAttemptEligible(MacAddress destination)
{
    if (!isDiscoveryActive(destination)) return false;
    if (!forwarding->hasPendingSource(destination)) { cancelDiscovery(destination); return false; }
    auto& discovery = discoveries.at(destination);
    if (!discovery.attempts || !discovery.sourceDue || simTime() < *discovery.sourceDue) return false;
    if (*discovery.attempts > maxRetries) {
        cancelDiscovery(destination); forwarding->discoveryFailed(destination); return false;
    }
    return !discovery.lastSent || simTime() >= *discovery.lastSent+preqMinInterval;
}

void Hwmp::recordPreqHandoff(MacAddress destination, bool sourceAttempt, bool publish)
{
    auto& discovery = discoveries.at(destination);
    discovery.lastSent = simTime();
    if (sourceAttempt) { (*discovery.attempts)++; discovery.sourceDue = simTime()+2*traversalTime; }
    if (publish && discovery.attempts) publishDiscovery(destination);
}

void Hwmp::sendSourcePreq(MacAddress destination)
{
    HwmpPreq preq;
    preq.setTtl(initialTtl);
    preq.setDiscoveryId(nextDiscoveryId++);
    preq.setOriginator(mib->address);
    preq.setOriginatorSequenceNumber(++ownSequence);
    preq.setLifetime(activePathTimeout.inUnit(SIMTIME_US)/1024);
    preq.setTargetsArraySize(1);
    HwmpPreqTarget target; target.address = destination;
    auto retained = paths.find(destination);
    if (retained != paths.end() && retained->second.sequence) target.sequenceNumber = *retained->second.sequence;
    else target.flags = 4; // USN=1: reserved target SN emitted as zero
    preq.setTargets(0,target);
    recordPreqHandoff(destination,true,false);
    sendPreq(preq,MacAddress::BROADCAST_ADDRESS);
    if (isDiscoveryActive(destination)) publishDiscovery(destination);
}

void Hwmp::handleMessageWhenUp(cMessage *message)
{
    if (message == perrTimer) { sendPendingErrors(); return; }
    if (message == rootOriginTimer) { originateRoot(); return; }
    if (message == rootWorkTimer) { processRootWork(); return; }
    if (message == pathExpiryTimer) {
        std::vector<MacAddress> expired;
        for (auto& entry : paths) if (entry.second.validated && simTime() >= entry.second.expiry) {
            entry.second.validated = false;
            expired.push_back(entry.first);
        }
        expireSeen();
        schedulePathExpiry();
        for (auto destination : expired) publishPath(destination);
        processRootWork();
        return;
    }
    for (const auto& entry : discoveries) if (entry.second.timer == message) {
        auto destination = entry.first;
        attemptDiscovery(destination);
        return;
    }
    throw cRuntimeError("Unknown HWMP timer");
}

void Hwmp::notifyUsable(MacAddress destination)
{
    if (findPath(destination)) {
        // Deferred, unhanded errors become obsolete at repair, even if this
        // path fails again before the next global PERR opportunity.
        pendingErrors.erase(std::remove_if(pendingErrors.begin(),pendingErrors.end(),[&](const PendingError& error) {
            return error.destination.address == destination;
        }),pendingErrors.end());
        if (pendingErrors.empty()) cancelEvent(perrTimer);
        cancelDiscovery(destination); // active protocol state gone before the synchronous drain
        forwarding->pathAvailable(destination);
    }
}

void Hwmp::sendPreq(const HwmpPreq& preq, MacAddress receiver)
{
    auto body = makeShared<Ieee80211HwmpFrame>();
    body->setPreqsArraySize(1); body->setPreqs(0,preq);
    body->setChunkLength(B(28+11*preq.getTargetsArraySize()));
    management->sendHwmp(body,receiver);
}
void Hwmp::sendPrep(const HwmpPrep& prep, MacAddress receiver)
{
    paths.at(prep.originator).validated = true; //14.11.8.4/Table14-10
    schedulePathExpiry();
    auto body = makeShared<Ieee80211HwmpFrame>();
    body->setPrepsArraySize(1); body->setPreps(0,prep); body->setChunkLength(B(33));
    management->sendHwmp(body,receiver);
}

void Hwmp::reply(const HwmpPreq& preq, const HwmpPreqTarget& target, bool local)
{
    auto reverse = paths.find(preq.getOriginator());
    if (reverse == paths.end() || simTime() >= reverse->second.expiry || !mib->isMeshPeerEstablished(reverse->second.nextHop)) return;
    HwmpPrep prep;
    prep.ttl = initialTtl; prep.target = target.address; prep.originator = preq.getOriginator();
    prep.originatorSequenceNumber = preq.getOriginatorSequenceNumber(); prep.lifetime = preq.getLifetime();
    if (local) {
        if (!(target.flags&4) && isNewer(target.sequenceNumber,ownSequence)) ownSequence = target.sequenceNumber;
        prep.targetSequenceNumber = ++ownSequence;
        // Derived native-data integration policy: final destination admits its reverse next hop.
        addPrecursor(mib->address,reverse->second.nextHop,simTime()+SimTime(int64_t(preq.getLifetime())*1024,SIMTIME_US));
    }
    else {
        auto& forward = paths.at(target.address);
        prep.targetSequenceNumber = *forward.sequence; prep.metric = forward.metric;
        addPrecursor(target.address,reverse->second.nextHop,forward.expiry);
        addPrecursor(preq.getOriginator(),forward.nextHop,reverse->second.expiry);
    }
    sendPrep(prep,reverse->second.nextHop); // both reply cases start Hop Count zero and copy PREQ Lifetime
}

void Hwmp::receivePreq(const HwmpPreq& preq, MacAddress transmitter, MacAddress receiver)
{
    // Model no-self-learning invariant; own sequence authority never comes from an echoed element.
    if (preq.getOriginator() == mib->address || stale(preq.getOriginator(),preq.getOriginatorSequenceNumber())) return;
    for (size_t i = 0; i < preq.getTargetsArraySize(); i++) {
        auto target = preq.getTargets(i);
        if (!mib->isMeshForwarding() && target.address != mib->address && !target.address.isBroadcast()) return;
        if ((preq.getFlags()&2) && target.address != mib->address && !findPath(target.address)) return;
    }
    bool individual = preq.getFlags()&2;
    auto identity = std::make_pair(preq.getOriginator(),preq.getDiscoveryId());
    bool previouslySeen = !individual && seen.count(identity);
    if (!individual && !previouslySeen && seen.size() >= size_t(seenCapacity)) return;
    std::map<MacAddress,std::optional<PathInfo>> before{{preq.getOriginator(),getPathInfo(preq.getOriginator())}};
    for (size_t i = 0; i < preq.getTargetsArraySize(); i++) before.emplace(preq.getTargets(i).address,getPathInfo(preq.getTargets(i).address));
    auto priorSelf = selfPrecursors;
    auto expiry = simTime()+SimTime(int64_t(preq.getLifetime())*1024,SIMTIME_US);
    if (!individual) seen[identity] = std::max(seen[identity],expiry);
    uint32_t metric = addMetric(preq.getMetric(),linkMetric->getMetric(transmitter));
    bool updated = updatePath(preq.getOriginator(),transmitter,preq.getOriginatorSequenceNumber(),metric,unsigned(preq.getHopCount())+1,expiry);
    auto retainedOriginator = paths.at(preq.getOriginator()).sequence;
    bool propagate = updated || (retainedOriginator && preq.getOriginatorSequenceNumber() == *retainedOriginator && !previouslySeen);
    std::vector<HwmpPreqTarget> remaining;
    for (size_t i = 0; i < preq.getTargetsArraySize(); i++) {
        auto target = preq.getTargets(i);
        if (target.address.isBroadcast()) {
            if (preq.getTargetsArraySize() == 1 && (target.flags&1)) {
                if (updated) {
                    auto& root = roots[preq.getOriginator()];
                    root.invitation = preq;
                    root.responseSent = false;
                    root.responseRequired |= bool(preq.getFlags()&4);
                }
                remaining.push_back(target);
            }
            continue;
        }
        auto retained = paths.find(target.address);
        auto targetSequence = target.address == mib->address ? std::optional<uint32_t>(ownSequence)
            : retained == paths.end() ? std::nullopt : retained->second.sequence;
        bool receipt = updated || (!(target.flags&4) && targetSequence
            && target.sequenceNumber == *targetSequence && !previouslySeen);
        bool local = receipt && target.address == mib->address;
        auto route = findPath(target.address);
        bool intermediate = receipt && !local && !(target.flags&1) && route && retained->second.sequence
            && ((target.flags&4) || target.sequenceNumber == *retained->second.sequence || isNewer(*retained->second.sequence,target.sequenceNumber));
        if (local || intermediate) reply(preq,target,local);
        if (!local) {
            if (intermediate) target.flags |= 1;
            if (target.flags&4) target.sequenceNumber = 0;
            remaining.push_back(target);
        }
    }
    if (propagate && mib->isMeshForwarding() && preq.getTtl() > 1 && preq.getHopCount() < 255 && !remaining.empty()) {
        auto forwarded = preq;
        forwarded.setHopCount(preq.getHopCount()+1); forwarded.setTtl(preq.getTtl()-1); forwarded.setMetric(metric);
        forwarded.setTargetsArraySize(remaining.size());
        for (size_t i = 0; i < remaining.size(); i++) forwarded.setTargets(i,remaining[i]);
        auto destination = MacAddress::BROADCAST_ADDRESS;
        if (!receiver.isMulticast() && (preq.getFlags()&2)) destination = findPath(remaining.front().address)->nextHop;
        sendPreq(forwarded,destination);
    }
    schedulePathExpiry();
    publishPathChanges(before,priorSelf);
    notifyUsable(preq.getOriginator());
}

void Hwmp::receivePrep(const HwmpPrep& prep, MacAddress transmitter)
{
    if (prep.target == mib->address || (!mib->isMeshForwarding() && prep.originator != mib->address)
            || stale(prep.target,prep.targetSequenceNumber)) return;
    std::map<MacAddress,std::optional<PathInfo>> before{{prep.target,getPathInfo(prep.target)},{prep.originator,getPathInfo(prep.originator)}};
    auto priorSelf = selfPrecursors;
    auto metric = addMetric(prep.metric,linkMetric->getMetric(transmitter));
    bool updated = updatePath(prep.target,transmitter,prep.targetSequenceNumber,metric,unsigned(prep.hopCount)+1,
        simTime()+SimTime(int64_t(prep.lifetime)*1024,SIMTIME_US));
    paths.at(prep.target).validated = simTime() < paths.at(prep.target).expiry; // validation never revives expired information
    schedulePathExpiry();
    if (prep.originator == mib->address) {
        // Symmetric derived self-admission policy permits immediate reverse traffic.
        addPrecursor(mib->address,transmitter,simTime()+SimTime(int64_t(prep.lifetime)*1024,SIMTIME_US));
    }
    else if (updated && prep.ttl > 1 && prep.hopCount < 255) {
        auto reverse = paths.find(prep.originator);
        if (reverse != paths.end() && reverse->second.sequence && simTime() < reverse->second.expiry
                && mib->isMeshPeerEstablished(reverse->second.nextHop)) {
            auto nextHop = reverse->second.nextHop;
            addPrecursor(prep.target,nextHop,paths.at(prep.target).expiry);
            addPrecursor(prep.originator,paths.at(prep.target).nextHop,paths.at(prep.originator).expiry);
            auto forwarded = prep; forwarded.hopCount++; forwarded.ttl--; forwarded.metric = metric;
            sendPrep(forwarded,nextHop);
        }
    }
    publishPathChanges(before,priorSelf);
    notifyUsable(prep.originator);
    notifyUsable(prep.target);
}

void Hwmp::processHwmp(const Ptr<const Ieee80211HwmpFrame>& body, MacAddress transmitter, MacAddress receiver)
{
    Enter_Method("processHwmp");
    if (operationalState != OPERATING || !mib->isMeshPeerEstablished(transmitter) || body->isIncorrect() || body->isIncomplete()) return;
    // Whole-body unsupported-format gate precedes every table/history mutation.
    for (size_t i = 0; i < body->getPreqsArraySize(); i++)
        if ((body->getPreqs(i).getFlags()&64) || body->getPreqs(i).getTtl() == 0) return;
    for (size_t i = 0; i < body->getPrepsArraySize(); i++)
        if ((body->getPreps(i).flags&64) || body->getPreps(i).ttl == 0) return;
    for (size_t i = 0; i < body->getPerrsArraySize(); i++)
        for (size_t j = 0; j < body->getPerrs(i).getDestinationsArraySize(); j++)
            if (body->getPerrs(i).getDestinations(j).flags&64) return;
    for (size_t i = 0; i < body->getRannsArraySize(); i++)
        if (body->getRanns(i).interval == 0) return; // no valid MIB interval/positive send schedule
    auto rootBefore = observeRootState();
    bool priorDeferral = std::exchange(rootWorkDeferred,true);
    expireSeen();
    for (size_t i = 0; i < body->getPreqsArraySize(); i++) receivePreq(body->getPreqs(i),transmitter,receiver);
    for (size_t i = 0; i < body->getPrepsArraySize(); i++) receivePrep(body->getPreps(i),transmitter);
    for (size_t i = 0; i < body->getPerrsArraySize(); i++) receivePerr(body->getPerrs(i),transmitter);
    for (size_t i = 0; i < body->getRannsArraySize(); i++) receiveRann(body->getRanns(i),transmitter);
    rootWorkDeferred = priorDeferral;
    if (!priorDeferral) processRootWork(rootBefore);
}

void Hwmp::sourcePathNeeded(MacAddress destination)
{
    Enter_Method("sourcePathNeeded");
    if (operationalState != OPERATING) return;
    auto before = observeRootState();
    auto root = roots.find(destination);
    if (root != roots.end() && root->second.invitation && !root->second.responseSent)
        root->second.responseRequired = true;
    processRootWork(before);
}

void Hwmp::clearRootWork()
{
    if (rootOriginTimer) cancelEvent(rootOriginTimer);
    cancelEvent(rootWorkTimer);
    localRannPending = false;
    for (auto& entry : roots) {
        auto& root = entry.second;
        root.invitation.reset(); root.responseSent = false; root.responseRequired = false;
        root.announcement.reset(); root.transmitter = MacAddress::UNSPECIFIED_ADDRESS; root.metric = UINT32_MAX;
        root.confirmationRequested.reset(); root.propagationRequested.reset();
    }
}

B Hwmp::rootBatchBudget() const
{
    // The admitted profile is entirely OFDM. Keep both the management Frame
    // Body authority and the PHY MPDU authority; DATA modes are not a substitute.
    auto header = makeShared<Ieee80211MeshActionFrame>();
    auto trailer = makeShared<Ieee80211MacTrailer>();
    B phyBody = B(physicallayer::Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz.getMpduMaxLength())
        - B(header->getChunkLength()) - B(trailer->getChunkLength());
    return std::min(MAX_MESH_ACTION_BODY_LENGTH,phyBody);
}

std::string Hwmp::describeRootWork() const
{
    std::ostringstream out;
    for (const auto& entry : roots) {
        out << entry.first;
        if (entry.second.confirmationRequested) out << " confirmationNotBefore=" << confirmationDue(entry.first);
        if (entry.second.propagationRequested) out << " RANNNotBefore=" << (lastRannAction ? *lastRannAction+rannInterval : simTime())
            << " usablePath=" << findPath(entry.first).has_value();
        out << "; ";
    }
    return out.str();
}

bool Hwmp::confirmationNeeded(MacAddress destination) const
{
    auto it = roots.find(destination);
    if (it == roots.end() || !it->second.announcement || !mib->isMeshPeerEstablished(it->second.transmitter)) return false;
    const auto& root = it->second;
    if (!findPath(destination) || root.metric < paths.at(destination).metric) return true;
    auto target = discoveries.find(destination);
    return target == discoveries.end() || !target->second.lastSent || simTime() >= *target->second.lastSent+confirmationInterval;
}

simtime_t Hwmp::confirmationDue(MacAddress destination) const
{
    const auto& root = roots.at(destination);
    auto due = root.confirmationRequested.value_or(SIMTIME_MAX);
    auto target = discoveries.find(destination);
    if (target != discoveries.end() && target->second.lastSent) due = std::max(due,*target->second.lastSent+preqMinInterval);
    if (lastConfirmationAction) due = std::max(due,*lastConfirmationAction+confirmationInterval);
    return due;
}

void Hwmp::schedulePreqWork(MacAddress destination)
{
    auto& target = discoveries.at(destination);
    cancelEvent(target.timer);
    simtime_t due = target.attempts ? getDiscoveryDeadline(destination) : SIMTIME_MAX;
    auto root = roots.find(destination);
    if (root != roots.end() && root->second.confirmationRequested && confirmationNeeded(destination))
        due = std::min(due,confirmationDue(destination));
    if (due != SIMTIME_MAX) {
        // Work already eligible is handled inline by the surrounding flush.
        // The positive inactive-owner cadence cannot create a same-time loop.
        if (due <= simTime()) due = simTime()+preqMinInterval;
        scheduleAt(due,target.timer);
    }
}

void Hwmp::receiveRann(const HwmpRann& rann, MacAddress transmitter)
{
    if (rann.root == mib->address) return;
    auto metric = addMetric(rann.metric,linkMetric->getMetric(transmitter));
    auto prior = roots.find(rann.root);
    if (prior != roots.end() && prior->second.announcement) {
        const auto& old = *prior->second.announcement;
        if ((rann.sequenceNumber != old.sequenceNumber && !isNewer(rann.sequenceNumber,old.sequenceNumber))
                || (rann.sequenceNumber == old.sequenceNumber && metric > prior->second.metric)) return;
    }
    auto& root = roots[rann.root];
    root.announcement = rann; root.transmitter = transmitter; root.metric = metric;
    if (!root.propagationRequested) root.propagationRequested = simTime();
    rannInterval = SimTime(int64_t(rann.interval)*1024,SIMTIME_US); // shared scalar; last accepted element wins
    if (confirmationNeeded(rann.root) && !root.confirmationRequested) root.confirmationRequested = simTime();
}

void Hwmp::sendProactivePrep(MacAddress address)
{
    auto it = roots.find(address);
    auto reverse = paths.find(address);
    if (it == roots.end() || !it->second.invitation || !it->second.responseRequired || it->second.responseSent
            || reverse == paths.end() || simTime() >= reverse->second.expiry || !mib->isMeshPeerEstablished(reverse->second.nextHop)
            || !management->isMeshManagementActive()) return;
    auto invitation = *it->second.invitation;
    auto nextHop = reverse->second.nextHop;
    auto before = getPathInfo(address); auto priorSelf = selfPrecursors;
    HwmpPrep prep; prep.ttl = initialTtl; prep.target = mib->address;
    // Selected interpretation of14.11.8.3/Table14-21: update the local responder
    // before each generated proactive reply; never use the broadcast reserved SN.
    prep.targetSequenceNumber = ++ownSequence;
    prep.originator = address; prep.originatorSequenceNumber = invitation.getOriginatorSequenceNumber(); prep.lifetime = invitation.getLifetime();
    addPrecursor(mib->address,nextHop,simTime()+SimTime(int64_t(prep.lifetime)*1024,SIMTIME_US));
    it->second.responseSent = true; it->second.responseRequired = false;
    sendPrep(prep,nextHop);
    publishPathChanges({{address,before},{mib->address,std::nullopt}},priorSelf);
    notifyUsable(address);
}

void Hwmp::scheduleRootOrigin()
{
    if (rootMode != RANN || !rootOriginTimer || operationalState != OPERATING) return;
    cancelEvent(rootOriginTimer);
    auto due = lastOwnRann.value_or(lastChange)+rannInterval;
    localRannPending = due <= simTime();
    if (!localRannPending) scheduleAt(due,rootOriginTimer);
}

void Hwmp::originateRoot()
{
    auto before = observeRootState();
    if (rootMode == RANN) { localRannPending = true; processRootWork(before); return; }
    if (rootMode == NO_ROOT || operationalState != OPERATING) return;
    if (management->isMeshManagementActive()) {
        HwmpPreq preq; preq.setFlags(rootMode == PREQ_WITH_PREP ? 4 : 0); preq.setTtl(initialTtl);
        preq.setDiscoveryId(nextDiscoveryId++); preq.setOriginator(mib->address); preq.setOriginatorSequenceNumber(++ownSequence);
        preq.setLifetime(activePathToRootTimeout.inUnit(SIMTIME_US)/1024); preq.setTargetsArraySize(1);
        HwmpPreqTarget target; target.flags = 5; target.address = MacAddress::BROADCAST_ADDRESS; preq.setTargets(0,target);
        lastRootPreq = simTime(); sendPreq(preq,MacAddress::BROADCAST_ADDRESS);
    }
    scheduleAfter(rootInterval,rootOriginTimer);
    publishRootChanges(before);
}

void Hwmp::sendRanns()
{
    if (!management->isMeshManagementActive() || (lastRannAction && simTime() < *lastRannAction+rannInterval)) return;
    std::vector<std::pair<simtime_t,MacAddress>> ready;
    if (localRannPending) ready.emplace_back(lastOwnRann.value_or(lastChange)+rannInterval,mib->address);
    for (const auto& entry : roots) {
        const auto& root = entry.second;
        if (root.announcement && root.propagationRequested && findPath(entry.first) && mib->isMeshForwarding()
                && root.announcement->ttl > 1 && root.announcement->hopCount < 255)
            ready.emplace_back(*root.propagationRequested,entry.first);
    }
    std::sort(ready.begin(),ready.end());
    auto body = makeShared<Ieee80211HwmpFrame>();
    B length(0);
    for (const auto& item : ready) {
        if (length+B(23) > rootBatchBudget()) break;
        HwmpRann rann;
        if (item.second == mib->address) {
            rann.ttl = initialTtl; rann.root = mib->address; rann.sequenceNumber = ++ownSequence;
            rann.interval = rannInterval.inUnit(SIMTIME_US)/1024;
            localRannPending = false; lastOwnRann = simTime();
        }
        else {
            auto& root = roots.at(item.second);
            rann = *root.announcement; rann.hopCount++; rann.ttl--; rann.metric = root.metric;
            root.propagationRequested.reset(); root.lastPropagation = simTime();
        }
        auto index = body->getRannsArraySize(); body->setRannsArraySize(index+1); body->setRanns(index,rann); length += B(23);
    }
    if (body->getRannsArraySize()) {
        body->setChunkLength(length); lastRannAction = simTime();
        management->sendHwmp(body,MacAddress::BROADCAST_ADDRESS);
        scheduleRootOrigin();
    }
}

void Hwmp::sendRootConfirmations()
{
    if (!management->isMeshManagementActive() || (lastConfirmationAction && simTime() < *lastConfirmationAction+confirmationInterval)) return;
    std::vector<std::pair<simtime_t,MacAddress>> ready;
    for (const auto& entry : roots)
        if (entry.second.confirmationRequested && confirmationNeeded(entry.first) && confirmationDue(entry.first) <= simTime())
            ready.emplace_back(*entry.second.confirmationRequested,entry.first);
    std::sort(ready.begin(),ready.end());
    if (ready.empty()) return;
    auto recipient = roots.at(ready.front().second).transmitter;
    auto body = makeShared<Ieee80211HwmpFrame>();
    B length(0);
    std::vector<std::pair<MacAddress,bool>> selected;
    for (const auto& item : ready) {
        auto address = item.second;
        if (roots.at(address).transmitter != recipient) continue;
        if (length+B(39) > rootBatchBudget()) break;
        bool sourceAttempt = sourceAttemptEligible(address);
        if (operationalState != OPERATING || !management->isMeshManagementActive()) return;
        selected.emplace_back(address,sourceAttempt);
        length += B(39);
    }
    length = B(0);
    std::vector<MacAddress> included;
    for (const auto& selection : selected) {
        auto address = selection.first;
        if (!confirmationNeeded(address) || roots.at(address).transmitter != recipient || confirmationDue(address) > simTime()) continue;
        const auto& announcement = *roots.at(address).announcement;
        HwmpPreq preq; preq.setFlags(2); preq.setTtl(initialTtl);
        preq.setOriginator(mib->address); preq.setOriginatorSequenceNumber(++ownSequence);
        preq.setLifetime(activePathToRootTimeout.inUnit(SIMTIME_US)/1024); preq.setTargetsArraySize(1);
        HwmpPreqTarget target; target.flags = 1; target.address = address; target.sequenceNumber = announcement.sequenceNumber;
        auto retained = paths.find(address);
        if (retained != paths.end() && retained->second.sequence && !isNewer(target.sequenceNumber,*retained->second.sequence))
            target.sequenceNumber = *retained->second.sequence;
        preq.setTargets(0,target); // unused Path Discovery ID remains zero and consumes no group ID
        roots.at(address).confirmationRequested.reset();
        recordPreqHandoff(address,selection.second,false);
        auto index = body->getPreqsArraySize(); body->setPreqsArraySize(index+1); body->setPreqs(index,preq);
        included.push_back(address); length += B(39);
    }
    if (included.empty()) return;
    body->setChunkLength(length); lastConfirmationAction = simTime();
    management->sendHwmp(body,recipient);
    for (auto address : included) if (isDiscoveryActive(address)) publishDiscovery(address);
}

void Hwmp::processRootWork(std::optional<RootObservation> before)
{
    if (rootWorkDeferred || operationalState != OPERATING) return;
    if (!before) before = observeRootState();
    bool priorDeferral = std::exchange(rootWorkDeferred,true);
    cancelEvent(rootWorkTimer);
    expireSeen();
    std::vector<MacAddress> sourceTargets;
    for (const auto& entry : discoveries) if (entry.second.attempts) sourceTargets.push_back(entry.first);
    for (auto destination : sourceTargets) {
        if (!forwarding->hasPendingSource(destination)) { cancelDiscovery(destination); continue; }
        auto& target = discoveries.at(destination);
        if (!management->isMeshManagementActive()
                || (target.sourceDue && simTime() >= *target.sourceDue && target.attempts && *target.attempts > maxRetries)) {
            cancelDiscovery(destination); forwarding->discoveryFailed(destination);
        }
    }
    std::vector<MacAddress> rootAddresses;
    for (const auto& entry : roots) rootAddresses.push_back(entry.first);
    for (auto address : rootAddresses) {
        {
            auto& root = roots.at(address);
            auto reverse = paths.find(address);
            if (root.invitation && (reverse == paths.end() || simTime() >= reverse->second.expiry || !mib->isMeshPeerEstablished(reverse->second.nextHop))) {
                root.invitation.reset(); root.responseRequired = false; root.responseSent = false;
            }
        }
        if (roots.at(address).invitation && !roots.at(address).responseSent && forwarding->hasPendingSource(address))
            roots.at(address).responseRequired = true;
        if (roots.at(address).invitation && roots.at(address).responseRequired && !roots.at(address).responseSent) sendProactivePrep(address);
        // Reacquire after readiness callbacks; no iterator is retained across them.
        if (confirmationNeeded(address)) {
            auto& current = roots.at(address);
            if (!current.confirmationRequested) current.confirmationRequested = simTime();
        }
        else roots.at(address).confirmationRequested.reset();
        if (roots.at(address).confirmationRequested) {
            auto& target = discoveries[address];
            if (!target.timer) target.timer = new cMessage("hwmpDiscovery");
        }
    }
    if (operationalState == OPERATING && management->isMeshManagementActive()) {
        sendRootConfirmations();
        for (auto destination : sourceTargets) {
            if (sourceAttemptEligible(destination)) sendSourcePreq(destination);
            else if (isDiscoveryActive(destination)) {
                auto& target = discoveries.at(destination);
                if (target.sourceDue && simTime() >= *target.sourceDue && target.lastSent && simTime() < *target.lastSent+preqMinInterval) {
                    target.sourceDue = *target.lastSent+preqMinInterval;
                    publishDiscovery(destination);
                }
            }
        }
        scheduleRootOrigin();
        sendRanns();
    }
    if (operationalState == OPERATING) {
        for (const auto& entry : discoveries) schedulePreqWork(entry.first);
        simtime_t next = SIMTIME_MAX;
        for (const auto& entry : roots) {
            const auto& root = entry.second;
            if (root.invitation) {
                auto reverse = paths.find(entry.first);
                if (reverse != paths.end() && !reverse->second.validated && simTime() < reverse->second.expiry) next = std::min(next,reverse->second.expiry);
                if (root.responseRequired && !root.responseSent && reverse != paths.end() && mib->isMeshPeerEstablished(reverse->second.nextHop))
                    next = std::min(next,simTime()+preqMinInterval);
            }
            auto target = discoveries.find(entry.first);
            if (root.announcement && findPath(entry.first) && !root.confirmationRequested && target != discoveries.end() && target->second.lastSent)
                next = std::min(next,*target->second.lastSent+confirmationInterval);
            if (root.announcement && root.propagationRequested && findPath(entry.first) && mib->isMeshForwarding()
                    && root.announcement->ttl > 1 && root.announcement->hopCount < 255)
                next = std::min(next,lastRannAction ? *lastRannAction+rannInterval : simTime());
        }
        if (localRannPending) next = std::min(next,lastRannAction ? *lastRannAction+rannInterval : simTime());
        if (next != SIMTIME_MAX) {
            if (next <= simTime()) next = simTime()+preqMinInterval;
            scheduleAt(next,rootWorkTimer);
        }
    }
    rootWorkDeferred = priorDeferral;
    if (!priorDeferral) publishRootChanges(*before);
}

Hwmp::RootObservation Hwmp::observeRootState() const
{
    RootObservation result;
    result.roots = roots;
    result.globals = {rannInterval,lastRootPreq,lastOwnRann,lastRannAction,lastConfirmationAction,localRannPending};
    for (const auto& entry : roots) {
        auto target = discoveries.find(entry.first);
        result.targetPreqs[entry.first] = target == discoveries.end() ? std::nullopt : target->second.lastSent;
        result.usable[entry.first] = findPath(entry.first).has_value();
    }
    return result;
}

void Hwmp::publishRootChanges(const RootObservation& before)
{
    auto after = observeRootState();
    std::set<MacAddress> addresses;
    for (const auto& entry : before.roots) addresses.insert(entry.first);
    for (const auto& entry : after.roots) addresses.insert(entry.first);
    bool globalChange = !(before.globals == after.globals);
    if (globalChange) addresses.insert(mib->address);
    auto rootAt = [](const auto& values, MacAddress address) -> std::optional<Root> {
        auto it = values.find(address); return it == values.end() ? std::nullopt : std::optional<Root>(it->second);
    };
    for (auto address : addresses) {
        auto old = rootAt(before.roots,address); auto current = rootAt(after.roots,address);
        auto priorPreq = before.targetPreqs.find(address); auto currentPreq = after.targetPreqs.find(address);
        auto priorTime = priorPreq == before.targetPreqs.end() ? std::nullopt : priorPreq->second;
        auto currentTime = currentPreq == after.targetPreqs.end() ? std::nullopt : currentPreq->second;
        bool oldUsable = before.usable.count(address) && before.usable.at(address);
        bool newUsable = after.usable.count(address) && after.usable.at(address);
        if (!globalChange && old == current && priorTime == currentTime && oldUsable == newUsable) continue;
        RootChange change(address,current,after.globals,currentTime,current && current->confirmationRequested
            ? std::optional<simtime_t>(confirmationDue(address)) : std::nullopt);
        emit(rootUpdatedSignal,newUsable,&change);
    }
}

void Hwmp::receivePerr(const HwmpPerr& perr, MacAddress transmitter)
{
    std::map<MacAddress,std::optional<PathInfo>> before;
    std::vector<PendingError> errors;
    for (size_t i = 0; i < perr.getDestinationsArraySize(); i++) {
        auto destination = perr.getDestinations(i);
        bool missing = destination.reasonCode == RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION;
        bool unreachable = destination.reasonCode == RC_MESH_PATH_ERROR_DESTINATION_UNREACHABLE;
        if (!missing && !unreachable) continue;
        auto it = paths.find(destination.address);
        if (it == paths.end()) continue;
        auto& path = it->second;
        if (!path.validated || !path.sequence || simTime() >= path.expiry || path.nextHop != transmitter) continue;
        if (missing && destination.sequenceNumber == 0)
            destination.sequenceNumber = *path.sequence+1;
        else if (!isNewer(destination.sequenceNumber,*path.sequence)
                && !(missing && destination.sequenceNumber == *path.sequence))
            continue;
        before.emplace(destination.address,path);
        path.sequence = destination.sequenceNumber;
        path.validated = false;
        if (mib->isMeshForwarding() && perr.getTtl() > 1)
            for (const auto& precursor : path.precursors)
                if (simTime() < precursor.second && mib->isMeshPeerEstablished(precursor.first))
                    errors.push_back({precursor.first,uint8_t(perr.getTtl()-1),destination,precursor.second});
    }
    schedulePathExpiry();
    publishPathChanges(before,selfPrecursors);
    queueErrors(errors);
}

void Hwmp::reportUnknownTransit(MacAddress transmitter, MacAddress destination)
{
    Enter_Method("reportUnknownTransit");
    if (operationalState != OPERATING || !mib->isMeshPeerEstablished(transmitter) || destination == mib->address
            || destination.isUnspecified() || destination.isMulticast() || findPath(destination)) return;
    HwmpPerrDestination entry;
    entry.address = destination; entry.reasonCode = RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION;
    // Case B has no route/precursor Lifetime. Established recipient, repair
    // and owner lifecycle bound its retention instead of an invented timeout.
    queueErrors({{transmitter,uint8_t(initialTtl),entry,SIMTIME_MAX}});
}

void Hwmp::queueErrors(const std::vector<PendingError>& errors)
{
    auto key = [](const PendingError& error) {
        const auto& destination = error.destination;
        return std::make_tuple(error.recipient,error.ttl,destination.address,destination.flags,destination.reasonCode,destination.sequenceNumber);
    };
    for (const auto& error : errors) {
        auto prior = std::find_if(pendingErrors.begin(),pendingErrors.end(),[&](const PendingError& pending) { return key(pending) == key(error); });
        if (prior == pendingErrors.end()) pendingErrors.push_back(error);
        else prior->expiry = std::max(prior->expiry,error.expiry);
    }
    sendPendingErrors();
}

void Hwmp::sendPendingErrors()
{
    cancelEvent(perrTimer);
    if (operationalState != OPERATING) return;
    pendingErrors.erase(std::remove_if(pendingErrors.begin(),pendingErrors.end(),[&](const PendingError& error) {
        return simTime() >= error.expiry || !mib->isMeshPeerEstablished(error.recipient) || findPath(error.destination.address).has_value();
    }),pendingErrors.end());
    if (!management->isMeshManagementActive()) { pendingErrors.clear(); return; }
    if (pendingErrors.empty()) return;
    if (lastPerrHandoff && simTime() < *lastPerrHandoff+perrMinInterval) {
        scheduleAt(*lastPerrHandoff+perrMinInterval,perrTimer);
        return;
    }
    std::sort(pendingErrors.begin(),pendingErrors.end(),[](const PendingError& a, const PendingError& b) {
        return std::tie(a.recipient,a.ttl,a.destination.address,a.destination.flags,a.destination.reasonCode,a.destination.sequenceNumber)
            < std::tie(b.recipient,b.ttl,b.destination.address,b.destination.flags,b.destination.reasonCode,b.destination.sequenceNumber);
    });
    auto recipient = pendingErrors.front().recipient;
    auto ttl = pendingErrors.front().ttl;
    std::set<MacAddress> selected;
    std::vector<PendingError> remaining;
    HwmpPerr perr; perr.setTtl(ttl);
    for (const auto& error : pendingErrors) {
        if (error.recipient == recipient && error.ttl == ttl && selected.size() < 19 && selected.insert(error.destination.address).second) {
            auto index = perr.getDestinationsArraySize();
            perr.setDestinationsArraySize(index+1); perr.setDestinations(index,error.destination);
        }
        else remaining.push_back(error);
    }
    pendingErrors = std::move(remaining); // remove notification ownership before calling another module
    auto body = makeShared<Ieee80211HwmpFrame>();
    body->setPerrsArraySize(1); body->setPerrs(0,perr); body->setChunkLength(B(4+13*perr.getDestinationsArraySize()));
    lastPerrHandoff = simTime();
    management->sendHwmp(body,recipient);
    if (operationalState == OPERATING && !pendingErrors.empty()) scheduleAt(*lastPerrHandoff+perrMinInterval,perrTimer);
}

} // namespace ieee80211
} // namespace inet
