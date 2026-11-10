// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/hwmp/Hwmp.h"

#include <algorithm>
#include <tuple>
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mesh/MeshDataAttempt.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Hwmp);
simsignal_t Hwmp::pathUpdatedSignal = registerSignal("hwmpPathUpdated");
simsignal_t Hwmp::discoveryChangedSignal = registerSignal("hwmpDiscoveryChanged");

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
        pathExpiryTimer = new cMessage("hwmpPathExpiry");
        perrTimer = new cMessage("hwmpPerr");
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
    }
}

Hwmp::~Hwmp()
{
    unsubscribeFailureNotifications();
    cancelAndDelete(pathExpiryTimer);
    cancelAndDelete(perrTimer);
    for (auto& entry : discoveries)
        cancelAndDelete(entry.second.timer);
}

void Hwmp::handleStartOperation(LifecycleOperation *)
{
    paths.clear(); selfPrecursors.clear(); seen.clear();
    mib->subscribe(Ieee80211Mib::meshPeerChangedSignal,this);
    hcf->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
}

void Hwmp::unsubscribeFailureNotifications()
{
    if (mib && mib->isSubscribed(Ieee80211Mib::meshPeerChangedSignal,this))
        mib->unsubscribe(Ieee80211Mib::meshPeerChangedSignal,this);
    if (hcf && hcf->isSubscribed(Hcf::meshDataAttemptCompletedSignal,this))
        hcf->unsubscribe(Hcf::meshDataAttemptCompletedSignal,this);
}

void Hwmp::receiveSignal(cComponent *, simsignal_t signal, bool value, cObject *details)
{
    Enter_Method_Silent();
    if (operationalState == OPERATING && signal == Ieee80211Mib::meshPeerChangedSignal && !value)
        invalidatePeer(check_and_cast<Ieee80211Mib::MeshPeerChange *>(details)->peer,true);
}

void Hwmp::receiveSignal(cComponent *, simsignal_t signal, cObject *, cObject *details)
{
    Enter_Method_Silent();
    if (operationalState == OPERATING && signal == Hcf::meshDataAttemptCompletedSignal) {
        auto attempt = check_and_cast<MeshDataAttempt *>(details);
        if (!attempt->acknowledged && attempt->terminalTimeout && mib->isMeshPeerEstablished(attempt->peer))
            invalidatePeer(attempt->peer,false);
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
    }
    paths.clear(); selfPrecursors.clear(); seen.clear();
    for (auto destination : removed) publishPath(destination);
    for (auto destination : failed) publishDiscovery(destination);
    for (auto destination : failed)
        forwarding->discoveryFailed(destination);
}
void Hwmp::handleStopOperation(LifecycleOperation *)
{
    unsubscribeFailureNotifications(); cancelEvent(perrTimer); pendingErrors.clear(); stopDiscoveries();
}
void Hwmp::handleCrashOperation(LifecycleOperation *)
{
    unsubscribeFailureNotifications(); cancelEvent(perrTimer); pendingErrors.clear(); stopDiscoveries();
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
    return it == discoveries.end() || !it->second.timer || !it->second.timer->isScheduled() ? SIMTIME_ZERO : it->second.timer->getArrivalTime();
}

void Hwmp::startDiscovery(MacAddress destination)
{
    Enter_Method("startDiscovery");
    if (operationalState != OPERATING || destination.isUnspecified() || destination.isMulticast() || destination == mib->address
            || isDiscoveryActive(destination)) return;
    auto& discovery = discoveries[destination];
    if (!discovery.timer) discovery.timer = new cMessage("hwmpDiscovery");
    discovery.attempts = 0;
    attemptDiscovery(destination);
}
void Hwmp::cancelDiscovery(MacAddress destination)
{
    Enter_Method("cancelDiscovery");
    auto it = discoveries.find(destination);
    if (it != discoveries.end()) {
        bool wasActive = it->second.attempts.has_value();
        cancelEvent(it->second.timer);
        it->second.attempts.reset(); // retain only minimum-interval history and reusable timer
        if (wasActive) publishDiscovery(destination);
    }
}

void Hwmp::attemptDiscovery(MacAddress destination)
{
    if (operationalState != OPERATING || !forwarding->hasPendingSource(destination)) {
        cancelDiscovery(destination);
        return;
    }
    if (!management->isMeshManagementActive()) {
        cancelDiscovery(destination);
        forwarding->discoveryFailed(destination);
        return;
    }
    expireSeen();
    auto& discovery = discoveries.at(destination);
    if (!discovery.attempts || operationalState != OPERATING) return;
    if (*discovery.attempts > maxRetries) {
        cancelDiscovery(destination);
        forwarding->discoveryFailed(destination);
        return;
    }
    if (discovery.lastSent && simTime() < *discovery.lastSent+preqMinInterval) {
        scheduleAt(*discovery.lastSent+preqMinInterval,discovery.timer);
        publishDiscovery(destination);
        return;
    }
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
    (*discovery.attempts)++;
    discovery.lastSent = simTime();
    scheduleAfter(2*traversalTime,discovery.timer);
    publishDiscovery(destination);
    sendPreq(preq,MacAddress::BROADCAST_ADDRESS);
}

void Hwmp::handleMessageWhenUp(cMessage *message)
{
    if (message == perrTimer) { sendPendingErrors(); return; }
    if (message == pathExpiryTimer) {
        std::vector<MacAddress> expired;
        for (auto& entry : paths) if (entry.second.validated && simTime() >= entry.second.expiry) {
            entry.second.validated = false;
            expired.push_back(entry.first);
        }
        expireSeen();
        schedulePathExpiry();
        for (auto destination : expired) publishPath(destination);
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
        if ((preq.getFlags()&2) && !findPath(target.address)) return;
    }
    auto identity = std::make_pair(preq.getOriginator(),preq.getDiscoveryId());
    bool previouslySeen = seen.count(identity);
    if (!previouslySeen && seen.size() >= size_t(seenCapacity)) return;
    std::map<MacAddress,std::optional<PathInfo>> before{{preq.getOriginator(),getPathInfo(preq.getOriginator())}};
    for (size_t i = 0; i < preq.getTargetsArraySize(); i++) before.emplace(preq.getTargets(i).address,getPathInfo(preq.getTargets(i).address));
    auto priorSelf = selfPrecursors;
    auto expiry = simTime()+SimTime(int64_t(preq.getLifetime())*1024,SIMTIME_US);
    seen[identity] = std::max(seen[identity],expiry);
    uint32_t metric = addMetric(preq.getMetric(),linkMetric->getMetric(transmitter));
    bool updated = updatePath(preq.getOriginator(),transmitter,preq.getOriginatorSequenceNumber(),metric,unsigned(preq.getHopCount())+1,expiry);
    auto retainedOriginator = paths.at(preq.getOriginator()).sequence;
    bool propagate = updated || (retainedOriginator && preq.getOriginatorSequenceNumber() == *retainedOriginator && !previouslySeen);
    std::vector<HwmpPreqTarget> remaining;
    for (size_t i = 0; i < preq.getTargetsArraySize(); i++) {
        auto target = preq.getTargets(i);
        if (target.address.isBroadcast()) continue; // proactive/root procedures arrive in C8
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
    expireSeen();
    for (size_t i = 0; i < body->getPreqsArraySize(); i++) receivePreq(body->getPreqs(i),transmitter,receiver);
    for (size_t i = 0; i < body->getPrepsArraySize(); i++) receivePrep(body->getPreps(i),transmitter);
    for (size_t i = 0; i < body->getPerrsArraySize(); i++) receivePerr(body->getPerrs(i),transmitter);
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
