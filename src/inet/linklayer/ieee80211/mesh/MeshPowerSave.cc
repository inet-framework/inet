// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/MeshPowerSave.h"

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"

namespace inet::ieee80211 {

Define_Module(MeshPowerSave);
simsignal_t MeshPowerSave::meshAwakeRequirementChangedSignal = cComponent::registerSignal("meshAwakeRequirementChanged");

MeshPowerSave::~MeshPowerSave()
{
    cancelAndDelete(boundaryTimer);
}

void MeshPowerSave::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        enabled = par("enabled");
        mib.reference(this, "mibModule", true);
        management.reference(this, "managementModule", true);
        buffers.reference(this, "hcfModule", true);
        mac.reference(this, "macModule", true);
        radioControl.reference(this, "macModule", true);
        wakeMargin = par("wakeMargin");
        if (wakeMargin < 0) throw cRuntimeError("Mesh wakeMargin must be nonnegative");
        boundaryTimer = new cMessage("meshAwakeBoundary");
        WATCH(deliveringGroups);
        WATCH_MAP(transitions);
        WATCH_MAP(awakeUntil);
        WATCH(awakeRequired);
        WATCH(ownRendezvous);
        WATCH(ownAwEnd);
        WATCH(postAwEnd);
        WATCH(maintenanceOrdinal);
        WATCH_MAP(peerWake);
    }
}

void MeshPowerSave::handleMessageWhenUp(cMessage *message)
{
    if (message != boundaryTimer) throw cRuntimeError("Unknown mesh awake boundary");
    radioControl->requestRadioReview();
}

void MeshPowerSave::preDelete(cComponent *root)
{
    if (root == this) {
        setOperationalState(NOT_OPERATING);
        clearPolicy();
    }
    // Whole-subtree teardown cannot call siblings from destruction.
    OperationalBase::preDelete(root);
}

void MeshPowerSave::handleStartOperation(LifecycleOperation *)
{
    if (enabled && !isActivationSupported())
        throw cRuntimeError("Mesh power save activation requires the complete mesh service-period engine");
    transitions.clear();
    awakeUntil.clear();
    deliveringGroups = false;
    if (enabled) radioControl->requestRadioReview();
}

void MeshPowerSave::clearPolicy()
{
    cancelEvent(boundaryTimer);
    ownAwEnd = postAwEnd = advertisedAw = SIMTIME_ZERO;
    groupIdleSince.reset();
    maintenanceOrdinal = 0;
    ownRendezvous = false;
    awakeRequired = true;
    peerWake.clear();
    auto old = transitions;
    transitions.clear();
    awakeUntil.clear();
    deliveringGroups = false;
    if (auto owner = radioControl.getNullable()) owner->requestRadioReview();
    for (const auto& entry : old)
        if (auto owner = buffers.getNullable())
            owner->withdrawMeshPowerControl(entry.second.originalId);
}

void MeshPowerSave::handleStopOperation(LifecycleOperation *) { clearPolicy(); }
void MeshPowerSave::handleCrashOperation(LifecycleOperation *) { clearPolicy(); }

MeshPeerInstance MeshPowerSave::instance(MacAddress peer) const
{
    return {management->getLocalLinkId(peer), management->getPeerLinkId(peer)};
}

MeshPowerMode MeshPowerSave::getRequestedMode(MacAddress peer) const
{
    auto pending = transitions.find(peer);
    if (pending != transitions.end())
        return pending->second.mode;
    auto state = mib->findMeshPeerPowerState(peer);
    return state ? state->local : MeshPowerMode::ACTIVE;
}

Packet *MeshPowerSave::makeModeIndication(MacAddress receiver, MeshPowerMode mode) const
{
    auto header = makeShared<Ieee80211MeshDataHeader>();
    header->setType(ST_QOS_NULL);
    header->setChunkLength(receiver.isMulticast() ? B(26) : B(32));
    header->setToDS(!receiver.isMulticast());
    header->setFromDS(true);
    header->setReceiverAddress(receiver);
    header->setTransmitterAddress(mib->address);
    header->setAddress3(receiver.isMulticast() ? mib->address : receiver);
    header->setAddress4(mib->address);
    header->setAckPolicy(receiver.isMulticast() ? NO_ACK : NORMAL_ACK);
    header->setEosp(true); // Mode-only indication, Table 14-33: no MPSP.
    header->setPowerMgmt(mode != MeshPowerMode::ACTIVE);
    header->setMeshPowerSaveLevel(mode == MeshPowerMode::DEEP_SLEEP);
    auto packet = new Packet("mesh-power-mode", header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
    return packet;
}

void MeshPowerSave::requestMode(MacAddress peer, MeshPowerMode mode)
{
    Enter_Method("requestMode");
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer))
        return;
    if (mode != MeshPowerMode::ACTIVE && mode != MeshPowerMode::LIGHT_SLEEP && mode != MeshPowerMode::DEEP_SLEEP)
        throw cRuntimeError("Invalid mesh power mode");
    auto committed = mib->findMeshPeerPowerState(peer)->local;
    auto old = transitions.find(peer);
    int64_t previous = old == transitions.end() ? -1 : old->second.originalId;
    auto group = transitions.find(MacAddress::BROADCAST_ADDRESS);
    int64_t previousGroup = group == transitions.end() ? -1 : group->second.originalId;
    transitions.erase(MacAddress::BROADCAST_ADDRESS);
    auto packet = std::unique_ptr<Packet>(makeModeIndication(peer, mode));
    auto id = packet->getId();
    auto peerInstance = instance(peer);
    // Publish the command correlation before any synchronous notification.
    // A nested command can supersede this never-enqueued local packet too.
    transitions[peer] = Transition{mode, peerInstance, id};
    // A higher activity level takes effect immediately, including supersession.
    if (int(mode) < int(committed))
        mib->commitMeshLocalPowerMode(peer, mode);
    // Retire the displaced original even if a notification replaced our command.
    if (previous != -1)
        buffers->withdrawMeshPowerControl(previous);
    if (previousGroup != -1)
        buffers->withdrawMeshPowerControl(previousGroup);
    auto current = transitions.find(peer);
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer) || current == transitions.end()
            || current->second.originalId != id || current->second.instance != peerInstance
            || instance(peer) != peerInstance)
        return;
    mac->sendMeshData(packet.release());
    buffers->meshPowerEligibilityChanged();
}

void MeshPowerSave::requestActiveForAllPeers()
{
    Enter_Method("requestActiveForAllPeers");
    if (!isEnabled())
        return;
    std::vector<std::pair<MacAddress, MeshPeerInstance>> peers;
    for (const auto& entry : mib->getMeshPeerPowerStates())
        peers.push_back({entry.first, instance(entry.first)});
    auto packet = std::unique_ptr<Packet>(makeModeIndication(MacAddress::BROADCAST_ADDRESS, MeshPowerMode::ACTIVE));
    auto id = packet->getId();
    auto old = transitions;
    transitions.clear();
    transitions.emplace(MacAddress::BROADCAST_ADDRESS, Transition{MeshPowerMode::ACTIVE, {}, id});
    mib->commitMeshLocalActiveForAllPeers();
    // A nested publication or retirement callback can replace this command,
    // but all originals detached by it must still be retired.
    for (const auto& entry : old)
        buffers->withdrawMeshPowerControl(entry.second.originalId);
    auto current = transitions.find(MacAddress::BROADCAST_ADDRESS);
    if (!isEnabled() || current == transitions.end() || current->second.originalId != id)
        return;
    for (const auto& peer : peers)
        if (!mib->isMeshPeerEstablished(peer.first) || instance(peer.first) != peer.second) {
            transitions.erase(current);
            return;
        }
    mac->sendMeshData(packet.release());
    buffers->meshPowerEligibilityChanged();
}

void MeshPowerSave::originalAcknowledged(const Packet *packet)
{
    Enter_Method("originalAcknowledged");
    auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    auto peer = header->getReceiverAddress();
    auto pending = transitions.find(peer);
    if (!isEnabled() || pending == transitions.end() || pending->second.originalId != packet->getId()
            || pending->second.instance != instance(peer) || !mib->isMeshPeerEstablished(peer))
        return;
    auto mode = pending->second.mode;
    transitions.erase(pending);
    mib->commitMeshLocalPowerMode(peer, mode);
    radioControl->requestRadioReview();
}

void MeshPowerSave::originalRetired(const Packet *packet)
{
    Enter_Method("originalRetired");
    for (auto it = transitions.begin(); it != transitions.end(); ++it)
        if (it->second.originalId == packet->getId()) {
            transitions.erase(it);
            break;
        }
    if (deliveringGroups && !hasBufferedGroup({}))
        deliveringGroups = false;
    if (isEnabled()) radioControl->requestRadioReview();
}

void MeshPowerSave::peerRemoved(MacAddress peer)
{
    Enter_Method("peerRemoved");
    auto pending = transitions.find(peer);
    int64_t id = pending == transitions.end() ? -1 : pending->second.originalId;
    transitions.erase(peer);
    awakeUntil.erase(peer);
    peerWake.erase(peer);
    radioControl->requestRadioReview();
    if (id != -1)
        buffers->withdrawMeshPowerControl(id);
}

bool MeshPowerSave::hasSleepingPeer() const
{
    for (const auto& entry : mib->getMeshPeerPowerStates())
        if (entry.second.remote != MeshPowerMode::ACTIVE) return true;
    return false;
}

bool MeshPowerSave::hasBufferedGroup(const std::set<int64_t>& exclude) const
{
    for (const auto& frame : buffers->getMeshBufferedFrames())
        if (frame.receiver.isMulticast() && frame.deadline > simTime() && !exclude.count(frame.id)
                && (frame.kind == MeshBufferedFrame::Kind::PAYLOAD || frame.kind == MeshBufferedFrame::Kind::MANAGEMENT))
            return true;
    return false;
}

bool MeshPowerSave::isTransmissionEligible(const Packet *packet, simtime_t start) const
{
    if (!isEnabled()) return true;
    auto header = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    if (!header) {
        auto bar = dynamicPtrCast<const Ieee80211BlockAckReq>(packet->peekAtFront<Ieee80211MacHeader>());
        if (!bar) return true; // Immediate recipient ACK/BA responses keep their MAC owner.
        auto state = mib->findMeshPeerPowerState(bar->getReceiverAddress());
        auto opportunity = awakeUntil.find(bar->getReceiverAddress());
        return !state || state->remote == MeshPowerMode::ACTIVE
            || (opportunity != awakeUntil.end() && start < opportunity->second);
    }
    auto type = header->getType();
    if (type == ST_BEACON || type == ST_PROBERESPONSE || type == ST_QOS_NULL)
        return true;
    if (header->getReceiverAddress().isMulticast())
        return !hasSleepingPeer() || deliveringGroups;
    if (deliveringGroups && hasBufferedGroup({}))
        return false;
    auto state = mib->findMeshPeerPowerState(header->getReceiverAddress());
    if (!state || state->remote == MeshPowerMode::ACTIVE) return true;
    auto opportunity = awakeUntil.find(header->getReceiverAddress());
    return opportunity != awakeUntil.end() && start < opportunity->second;
}

void MeshPowerSave::openAwakeOpportunity(MacAddress peer, simtime_t end)
{
    awakeUntil[peer] = end;
    buffers->meshPowerEligibilityChanged();
    radioControl->requestRadioReview();
}

void MeshPowerSave::prepareTransmission(Packet *packet, const std::set<int64_t>& deliveryUnit)
{
    Enter_Method("prepareTransmission");
    if (!isEnabled()) return;
    if (!dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(packet->peekAtFront<Ieee80211MacHeader>())) return;
    auto inventory = buffers->getMeshBufferedFrames();
    auto header = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
    auto peer = header->getReceiverAddress();
    auto state = mib->findMeshPeerPowerState(peer);
    auto mode = state ? state->local : mib->getMeshNonpeerPowerMode();
    auto pending = transitions.find(peer);
    // This profile selects one Null as the lower-mode indication. Unrelated
    // DATA/MMPDU ACKs cannot commit it, so those units advertise committed mode.
    if (header->getType() == ST_QOS_NULL && !peer.isMulticast() && pending != transitions.end()
            && pending->second.originalId == packet->getId() && pending->second.instance == instance(peer))
        mode = pending->second.mode;
    header->setPowerMgmt(mode != MeshPowerMode::ACTIVE);
    if (auto mesh = dynamicPtrCast<Ieee80211MeshDataHeader>(header)) {
        mesh->setMeshPowerSaveLevel(peer.isMulticast() ? mib->getMeshGroupPowerSaveLevel() : mode == MeshPowerMode::DEEP_SLEEP);
        mesh->setRspi(false);
    }
    bool more = false;
    for (const auto& frame : inventory)
        if (!deliveryUnit.count(frame.id) && frame.id != packet->getId()
                && frame.deadline > simTime()
                && (frame.kind == MeshBufferedFrame::Kind::PAYLOAD || frame.kind == MeshBufferedFrame::Kind::MANAGEMENT)
                && (peer.isMulticast() ? frame.receiver.isMulticast() :
                    state && state->remote != MeshPowerMode::ACTIVE && frame.receiver == peer))
            more = true;
    header->setMoreData(more);
    packet->insertAtFront(header);
}

void MeshPowerSave::receivePowerFields(const Packet *, const Ptr<const Ieee80211MeshDataHeader>& header)
{
    Enter_Method("receivePowerFields");
    auto peer = header->getTransmitterAddress();
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    auto mode = !header->getPowerMgmt() ? MeshPowerMode::ACTIVE :
        header->getMeshPowerSaveLevel() ? MeshPowerMode::DEEP_SLEEP : MeshPowerMode::LIGHT_SLEEP;
    auto state = mib->findMeshPeerPowerState(peer);
    // Group indications can announce higher activity only (14.15.3.2).
    if (!header->getReceiverAddress().isMulticast() || int(mode) < int(state->remote))
        mib->commitMeshRemotePowerMode(peer, mode);
    buffers->meshPowerEligibilityChanged();
    radioControl->requestRadioReview();
}

MeshTim MeshPowerSave::getTim(uint8_t count, uint8_t period, const std::set<int64_t>& exclude) const
{
    MeshTim tim;
    tim.dtimCount = count;
    tim.dtimPeriod = period;
    std::fill(std::begin(tim.partialVirtualBitmap), std::end(tim.partialVirtualBitmap), 0);
    std::array<uint8_t, 251> bitmap{};
    bool group = false;
    for (const auto& frame : buffers->getMeshBufferedFrames()) {
        if (exclude.count(frame.id) || frame.deadline <= simTime() || frame.kind == MeshBufferedFrame::Kind::TIMING
                || frame.kind == MeshBufferedFrame::Kind::POWER_CONTROL || frame.kind == MeshBufferedFrame::Kind::PROBE_RESPONSE)
            continue;
        if (frame.receiver.isMulticast()) { group = true; continue; }
        auto aid = management->getLocalAid(frame.receiver);
        if (aid && aid <= 2007) bitmap[aid / 8] |= 1 << (aid % 8);
    }
    int first = 0, last = 250;
    while (first < 250 && !bitmap[first]) ++first;
    while (last > first && !bitmap[last]) --last;
    if (!bitmap[first]) first = last = 0;
    first &= ~1;
    tim.bitmapControl = first | ((count == 0 && group) ? 1 : 0);
    tim.bitmapLength = last - first + 1;
    std::copy(bitmap.begin() + first, bitmap.begin() + last + 1, tim.partialVirtualBitmap);
    return tim;
}

bool MeshPowerSave::needsAwakeWindow(const MeshTim& tim) const
{
    if (tim.dtimCount == 0) return true;
    if (mib->getMeshNonpeerPowerMode() == MeshPowerMode::ACTIVE) return false;
    for (int i = 0; i < tim.bitmapLength; ++i)
        if (tim.partialVirtualBitmap[i]) return true;
    return false;
}

void MeshPowerSave::dtimTransmissionComplete()
{
    Enter_Method("dtimTransmissionComplete");
    if (isEnabled()) deliveringGroups = hasBufferedGroup({});
    buffers->meshPowerEligibilityChanged();
    radioControl->requestRadioReview();
}

void MeshPowerSave::groupTransmissionComplete(const Packet *packet)
{
    Enter_Method("groupTransmissionComplete");
    auto current = transitions.find(MacAddress::BROADCAST_ADDRESS);
    if (current != transitions.end() && current->second.originalId == packet->getId())
        transitions.erase(current);
    if (deliveringGroups && !hasBufferedGroup({packet->getId()})) deliveringGroups = false;
    radioControl->requestRadioReview();
}

void MeshPowerSave::managementTimingChanged()
{
    Enter_Method("managementTimingChanged");
    if (isEnabled()) radioControl->requestRadioReview();
}

void MeshPowerSave::radioObservationChanged()
{
    Enter_Method("radioObservationChanged");
    // Preserve intervening busy/nonreceptive observations even when their
    // same-time continuations coalesce. This does not command the radio inline.
    if (!radioControl->isMeshPhyIdle()) groupIdleSince.reset();
}

void MeshPowerSave::timingTransmissionComplete(simtime_t window)
{
    Enter_Method("timingTransmissionComplete");
    if (!isEnabled()) return;
    ownRendezvous = false;
    if (window > 0) {
        advertisedAw = window;
        ownAwEnd = std::max(ownAwEnd, simTime() + window);
    }
    radioControl->requestRadioReview();
}

void MeshPowerSave::timingTransmissionRetired()
{
    Enter_Method("timingTransmissionRetired");
    ownRendezvous = false;
    if (isEnabled()) radioControl->requestRadioReview();
}

void MeshPowerSave::beaconReceived(MacAddress peer, bool group, bool individual, simtime_t window)
{
    Enter_Method("beaconReceived");
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    auto& wake = peerWake[peer];
    wake.rendezvous = false;
    wake.groupBuffered = group;
    wake.individualBuffered = individual;
    if (window > 0) {
        awakeUntil[peer] = std::max(awakeUntil[peer], simTime() + window);
        buffers->meshPowerEligibilityChanged();
    }
    auto schedule = management->getMeshWakeSchedule();
    const auto& peers = mib->getMeshPeerPowerStates();
    auto wakeTime = radioControl->getMeshRadioSwitchingTime(physicallayer::IRadio::RADIO_MODE_SLEEP,
        physicallayer::IRadio::RADIO_MODE_TRANSCEIVER);
    if (!peers.empty() && peer == peers.begin()->first && maintenanceOrdinal
            && management->getPeerTbtt(peer, simTime())
            && wakeTime && (maintenanceOrdinal < schedule.ordinal
                || simTime() >= management->getOwnTbtt(maintenanceOrdinal)
                    - *wakeTime - wakeMargin - SimTime::fromRaw(1))) {
        // Keep the selected ten-local-period cadence anchored to its existing
        // ordinal. A long wake lead can accept a reference before that ordinal;
        // compare before subtracting the unsigned next-beacon counters.
        auto elapsed = schedule.ordinal > maintenanceOrdinal ? schedule.ordinal - maintenanceOrdinal : uint64_t(0);
        maintenanceOrdinal += (elapsed / 10 + 1) * 10;
    }
    if (group) groupIdleSince.reset();
    radioControl->requestRadioReview();
}

void MeshPowerSave::groupFrameReceived(MacAddress peer, bool moreData)
{
    Enter_Method("groupFrameReceived");
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    peerWake[peer].groupBuffered = moreData;
    radioControl->requestRadioReview();
}

void MeshPowerSave::ppduTransmissionComplete(const Packet *packet, simtime_t start)
{
    Enter_Method("ppduTransmissionComplete");
    if (!isEnabled()) return;
    auto protocol = packet->findTag<PacketProtocolTag>();
    if (protocol && protocol->getProtocol() == &Protocol::unknown) return; // Individual A-MPDU.
    auto header = packet->peekAtFront<Ieee80211MacHeader>();
    // IEEE Std 802.11-2024, 14.15.6 extends the awake window itself. The
    // selected recurrence includes its timed PostAW extension, qualifies by
    // actual START, and adds the nominal advertised duration at completion.
    // Pending DTIM delivery is a separate awake obligation, not a timed AW.
    auto activeWindowEnd = std::max(ownAwEnd, postAwEnd);
    if (header->getReceiverAddress().isMulticast() && header->getType() != ST_BEACON
            && header->getType() != ST_PROBERESPONSE && start >= 0 && start < activeWindowEnd)
        postAwEnd = std::max(postAwEnd, simTime() + advertisedAw);
    radioControl->requestRadioReview();
}

void MeshPowerSave::reviewAwakeState()
{
    Enter_Method("reviewAwakeState");
    if (!isEnabled()) return;
    using Radio = physicallayer::IRadio;
    auto wakeTime = radioControl->getMeshRadioSwitchingTime(Radio::RADIO_MODE_SLEEP, Radio::RADIO_MODE_TRANSCEIVER);
    auto sleepTime = radioControl->getMeshRadioSwitchingTime(Radio::RADIO_MODE_TRANSCEIVER, Radio::RADIO_MODE_SLEEP);
    if (!wakeTime || !sleepTime)
        throw cRuntimeError("Mesh doze requires initialized public radio transition observations");
    auto now = simTime();
    auto tick = SimTime::fromRaw(1);
    auto lead = *wakeTime + wakeMargin + tick;
    auto schedule = management->getMeshWakeSchedule();
    auto oldBoundary = boundaryTimer->isScheduled() ? boundaryTimer->getArrivalTime() : SimTime::getMaxTime();
    auto next = SimTime::getMaxTime();
    bool awake = schedule.peeringPending || schedule.beaconPending || radioControl->hasMeshRadioActivity()
        || !transitions.empty() || mib->getMeshNonpeerPowerMode() == MeshPowerMode::ACTIVE;
    auto boundary = [&](simtime_t time) { if (time > now) next = std::min(next, time); };
    if (schedule.nextTbtt >= now) {
        auto wakeAt = schedule.nextTbtt - lead;
        if (now >= wakeAt) ownRendezvous = true;
        else boundary(wakeAt);
    }
    awake |= ownRendezvous;
    for (auto end : {ownAwEnd, postAwEnd}) {
        awake |= now < end;
        boundary(end);
    }
    for (const auto& entry : awakeUntil) {
        awake |= now < entry.second;
        boundary(entry.second);
    }
    // A BU that is currently eligible can initiate a transmission. Sleeping-
    // peer BUs without an opportunity remain buffered and do not prevent doze.
    for (const auto& frame : buffers->getMeshBufferedFrames()) {
        if (frame.deadline <= now || frame.kind == MeshBufferedFrame::Kind::TIMING) continue;
        if (frame.kind == MeshBufferedFrame::Kind::PROBE_RESPONSE) {
            awake = true; // Timing response waits for its own actual admission, not a peer AW.
            continue;
        }
        if (frame.receiver.isMulticast()) awake |= !hasSleepingPeer() || deliveringGroups;
        else {
            auto state = mib->findMeshPeerPowerState(frame.receiver);
            auto opportunity = awakeUntil.find(frame.receiver);
            awake |= !state || state->remote == MeshPowerMode::ACTIVE
                || (opportunity != awakeUntil.end() && now < opportunity->second)
                || frame.kind == MeshBufferedFrame::Kind::POWER_CONTROL;
        }
    }
    const auto& peers = mib->getMeshPeerPowerStates();
    if (!maintenanceOrdinal) maintenanceOrdinal = schedule.ordinal + 10;
    // The ordinal denotes the NEXT scheduled beacon, not a period already
    // reached. A strictly later ordinal proves the target TBTT has passed;
    // equality still requires management's absolute target and wake lead.
    bool maintenanceDue = maintenanceOrdinal < schedule.ordinal
        || now >= management->getOwnTbtt(maintenanceOrdinal) - lead;
    if (!maintenanceDue && !peers.empty()) boundary(management->getOwnTbtt(maintenanceOrdinal) - lead);
    for (const auto& entry : peers) {
        auto& wake = peerWake[entry.first];
        awake |= entry.second.local == MeshPowerMode::ACTIVE;
        if (entry.second.local == MeshPowerMode::ACTIVE) wake.individualBuffered = false;
        bool required = entry.second.local == MeshPowerMode::LIGHT_SLEEP
            || (maintenanceDue && entry.first == peers.begin()->first);
        if (entry.second.local == MeshPowerMode::DEEP_SLEEP && entry.first == peers.begin()->first
                && !management->getPeerTbtt(entry.first, now + tick))
            awake = true; // Keep reacquiring an invalid selected deep reference.
        if (required) {
            auto tbtt = management->getPeerTbtt(entry.first, now + tick);
            if (!tbtt) awake = true; // Reacquire an absent/invalid reference while awake.
            else if (now >= *tbtt - lead) wake.rendezvous = true;
            else boundary(*tbtt - lead);
        }
        awake |= wake.rendezvous || wake.individualBuffered;
    }
    // IEEE Std 802.11-2024, 14.15.5: continuous PHY CCA idle, independent
    // of NAV/TXNAV and of the user's configured VI TXOP limit.
    bool groupWaiting = false;
    for (const auto& entry : peerWake) groupWaiting |= entry.second.groupBuffered;
    if (groupWaiting && radioControl->isMeshPhyIdle()) {
        if (!groupIdleSince) groupIdleSince = now;
        auto end = *groupIdleSince + buffers->getGroupDeliveryIdleTime();
        if (now >= end) {
            for (auto& entry : peerWake) entry.second.groupBuffered = false;
            groupIdleSince.reset();
        }
        else { awake = true; boundary(end); }
    }
    else {
        groupIdleSince.reset();
        awake |= groupWaiting;
    }
    // A sleep/wake round trip must leave a positive useful sleep interval.
    if (next != SimTime::getMaxTime() && now + *sleepTime >= next) awake = true;
    cancelEvent(boundaryTimer);
    if (next != SimTime::getMaxTime()) scheduleAt(next, boundaryTimer);
    bool changed = awakeRequired != awake || oldBoundary != next;
    awakeRequired = awake;
    if (changed) emit(meshAwakeRequirementChangedSignal, awakeRequired);
}

} // namespace inet::ieee80211
