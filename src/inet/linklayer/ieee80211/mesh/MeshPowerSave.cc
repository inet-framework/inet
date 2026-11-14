// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/MeshPowerSave.h"

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshPowerResponseTag_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

namespace inet::ieee80211 {

Define_Module(MeshPowerSave);
Register_Abstract_Class(MeshPowerSave::PeriodOutcome);
simsignal_t MeshPowerSave::meshAwakeRequirementChangedSignal = cComponent::registerSignal("meshAwakeRequirementChanged");
simsignal_t MeshPowerSave::meshServicePeriodOutcomeSignal = cComponent::registerSignal("meshServicePeriodOutcome");

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
        missingAckRetryLimit = par("missingAckRetryLimit");
        if (missingAckRetryLimit < 0 || missingAckRetryLimit > 100)
            throw cRuntimeError("Mesh missingAckRetryLimit must be0..100");
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
    periods.clear();
    triggers.clear();
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
    std::set<int64_t> controls;
    for (const auto& entry : old) controls.insert(entry.second.originalId);
    for (const auto& entry : triggers)
        if (entry.second.queuedId != -1) controls.insert(entry.second.queuedId);
    for (const auto& entry : periods)
        for (const auto& period : entry.second)
            if (period && period->endingNull != -1) controls.insert(period->endingNull);
    periods.clear();
    triggers.clear();
    transitions.clear();
    awakeUntil.clear();
    deliveringGroups = false;
    if (auto owner = radioControl.getNullable()) owner->requestRadioReview();
    for (auto id : controls)
        if (auto owner = buffers.getNullable())
            owner->withdrawMeshPowerControl(id);
}

MeshPowerSave::Period *MeshPowerSave::findPeriod(MacAddress peer, MeshPeriodDirection direction)
{
    auto it = periods.find(peer);
    return it != periods.end() && it->second[int(direction)] ? &*it->second[int(direction)] : nullptr;
}

const MeshPowerSave::Period *MeshPowerSave::findPeriod(MacAddress peer, MeshPeriodDirection direction) const
{
    auto it = periods.find(peer);
    return it != periods.end() && it->second[int(direction)] ? &*it->second[int(direction)] : nullptr;
}

void MeshPowerSave::openPeriod(MacAddress peer, MeshPeriodDirection direction, int64_t id, simtime_t time)
{
    auto state = mib->findMeshPeerPowerState(peer);
    if (!state || (direction == MeshPeriodDirection::OWNER ? state->remote : state->local) == MeshPowerMode::ACTIVE
            || findPeriod(peer, direction))
        return;
    Period period;
    period.instance = instance(peer);
    period.openingId = id;
    period.openingTime = time;
    period.cap = missingAckRetryLimit;
    periods[peer][int(direction)] = std::move(period);
    if (direction == MeshPeriodDirection::RECIPIENT)
        peerWake[peer].retrievalRequested = false;
}

std::optional<MeshPowerSave::Period> MeshPowerSave::detachRecipient(MacAddress peer)
{
    auto it = periods.find(peer);
    if (it == periods.end() || !it->second[int(MeshPeriodDirection::RECIPIENT)]) return {};
    auto old = std::move(it->second[int(MeshPeriodDirection::RECIPIENT)]);
    it->second[int(MeshPeriodDirection::RECIPIENT)].reset();
    if (!it->second[0] && !it->second[1]) periods.erase(it);
    return old;
}

void MeshPowerSave::retirePeriod(MacAddress peer, MeshPeriodDirection direction, MeshPeriodOutcome outcome)
{
    auto it = periods.find(peer);
    if (it == periods.end() || !it->second[int(direction)]) return;
    auto old = *it->second[int(direction)];
    it->second[int(direction)].reset();
    if (!it->second[0] && !it->second[1]) periods.erase(it);
    if (outcome == MeshPeriodOutcome::FAILED && !triggers.count(peer) && mib->isMeshPeerEstablished(peer)) {
        Trigger failed;
        failed.instance = instance(peer);
        failed.failed = true;
        triggers.emplace(peer, failed);
    }
    // Detach before callbacks; an observer may close/reopen the relationship.
    if (outcome != MeshPeriodOutcome::CONFIRMED && old.endingNull != -1)
        buffers->withdrawMeshPowerControl(old.endingNull);
    PeriodOutcome fact(peer, direction, outcome, old.openingId, old.openingTime);
    emit(meshServicePeriodOutcomeSignal, &fact);
    radioControl->requestRadioReview();
}

void MeshPowerSave::applyTrigger(MacAddress peer, bool rspi, bool eosp, int64_t id, simtime_t time)
{
    // Received-trigger roles, Table14-33. Sender roles use its own ACK below.
    if (!eosp) openPeriod(peer, MeshPeriodDirection::RECIPIENT, id, time);
    if (rspi) openPeriod(peer, MeshPeriodDirection::OWNER, id, time);
}

bool MeshPowerSave::peerAwake(MacAddress peer, simtime_t start) const
{
    auto state = mib->findMeshPeerPowerState(peer);
    auto window = awakeUntil.find(peer);
    return !state || state->remote == MeshPowerMode::ACTIVE || findPeriod(peer, MeshPeriodDirection::OWNER)
        || findPeriod(peer, MeshPeriodDirection::RECIPIENT) || (window != awakeUntil.end() && start < window->second);
}

bool MeshPowerSave::canInitiateTo(MacAddress peer, simtime_t start) const
{
    auto state = mib->findMeshPeerPowerState(peer);
    if (!state || !peerAwake(peer, start)) return false;
    auto window = awakeUntil.find(peer);
    return state->local == MeshPowerMode::LIGHT_SLEEP || state->remote == MeshPowerMode::ACTIVE
        || (window != awakeUntil.end() && start < window->second);
}

std::vector<MeshServicePeriodView> MeshPowerSave::getServicePeriods() const
{
    std::vector<MeshServicePeriodView> result;
    for (const auto& entry : periods)
        for (auto direction : {MeshPeriodDirection::OWNER, MeshPeriodDirection::RECIPIENT})
            if (auto& period = entry.second[int(direction)])
                result.push_back({entry.first, direction, period->ending, period->cap, period->spent, period->cohort});
    return result;
}

void MeshPowerSave::recipientMemberReceived(const Packet *packet,
        const Ptr<const Ieee80211MeshDataHeader>& header, bool duplicate)
{
    Enter_Method("recipientMemberReceived");
    receivePowerFields(packet, header);
    auto peer = header->getTransmitterAddress();
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer) || header->getReceiverAddress().isMulticast()) return;
    auto state = *mib->findMeshPeerPowerState(peer);
    if (state.remote == MeshPowerMode::ACTIVE)
        retirePeriod(peer, MeshPeriodDirection::OWNER, MeshPeriodOutcome::WITHDRAWN);
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    if (mib->findMeshPeerPowerState(peer)->local == MeshPowerMode::ACTIVE)
        retirePeriod(peer, MeshPeriodDirection::RECIPIENT, MeshPeriodOutcome::WITHDRAWN);
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer) || header->getAckPolicy() != NORMAL_ACK) return;
    // A PPDU admitted during our AW remains eligible if it completes later.
    auto time = packet->findTag<SignalTimeInd>();
    bool inOwnWindow = time && time->getStartTime() >= SIMTIME_ZERO && time->getStartTime() <= simTime()
        && time->getStartTime() < std::max(ownAwEnd, postAwEnd);
    if (state.local == MeshPowerMode::ACTIVE || state.remote == MeshPowerMode::LIGHT_SLEEP || inOwnWindow)
        applyTrigger(peer, header->getRspi(), header->getEosp(), packet->getId(), simTime());
    if (header->getEosp()) {
        if (auto period = findPeriod(peer, MeshPeriodDirection::RECIPIENT)) {
            period->ending = true;
            period->pendingReceipt = packet->getId();
            if (header->getType() == ST_DATA_WITH_QOS)
                period->pendingEosp.insert({header->getTid(), header->getSequenceNumber().get()});
        }
    }
    peerWake[peer].individualBuffered = header->getMoreData();
    buffers->meshPowerEligibilityChanged();
    radioControl->requestRadioReview();
}

bool MeshPowerSave::canTransmitSet(const std::vector<MeshPowerTxMember>& members, simtime_t start) const
{
    if (!isEnabled()) return true;
    int needed = 0;
    const Period *ending = nullptr;
    int cap = missingAckRetryLimit;
    int spent = 0;
    for (const auto& member : members) {
        if (member.peer.isMulticast()) continue;
        bool data = member.type == ST_DATA_WITH_QOS;
        bool null = member.type == ST_QOS_NULL;
        auto state = mib->findMeshPeerPowerState(member.peer);
        if (!state) {
            // Peering MMPDUs precede the established peer power view.
            if (data || null) return false;
            continue;
        }
        auto owner = findPeriod(member.peer, MeshPeriodDirection::OWNER);
        bool wantsOwner = !member.eosp && state->remote != MeshPowerMode::ACTIVE && !owner;
        bool wantsRecipient = member.rspi && state->local != MeshPowerMode::ACTIVE
            && !findPeriod(member.peer, MeshPeriodDirection::RECIPIENT);
        if ((data || null) && (wantsOwner || wantsRecipient) && !canInitiateTo(member.peer, start))
            return false;
        auto trigger = triggers.find(member.peer);
        if (data && state->remote != MeshPowerMode::ACTIVE && !owner
                && trigger != triggers.end() && trigger->second.failed)
            return false; // An old AW and unchanged inventory are not a fresh request.
        if (!owner && trigger != triggers.end() && !trigger->second.failed && trigger->second.queuedId != -1
                && member.id != trigger->second.queuedId)
            return false;
        if (owner && owner->ending) {
            if (!owner->cohort.count(member.id) && member.id != owner->endingNull) return false;
            ending = owner;
            cap = owner->cap;
            spent = owner->spent;
            if (owner->started.count(member.id)) needed++;
        }
        else if (owner && member.eosp) {
            ending = owner;
            cap = std::min(cap, member.ordinaryLimit);
            if (owner->started.count(member.id)) needed++;
        }
        else if (state->remote != MeshPowerMode::ACTIVE && !owner) {
            if (!peerAwake(member.peer, start)) return false;
        }
    }
    return !ending || needed <= (spent >= cap ? 0 : cap-spent);
}

bool MeshPowerSave::authorizeTransmissionStart(const std::vector<MeshPowerTxMember>& members, simtime_t start)
{
    Enter_Method("authorizeTransmissionStart");
    if (!isEnabled()) return true;
    if (!canTransmitSet(members, start)) return false;
    for (const auto& member : members) {
        if (member.peer.isMulticast()) continue;
        auto owner = findPeriod(member.peer, MeshPeriodDirection::OWNER);
        if (owner && member.eosp && !owner->ending) {
            owner->ending = true;
            owner->cap = std::min(missingAckRetryLimit, member.ordinaryLimit);
            for (const auto& covered : members)
                if (covered.peer == member.peer && covered.eosp) {
                    owner->cohort.insert(covered.id);
                    owner->cap = std::min(owner->cap, covered.ordinaryLimit);
                    if (covered.type == ST_QOS_NULL) owner->endingNull = covered.id;
                }
        }
    }
    if (!members.empty()) {
        const auto& first = members.front();
        if (!first.peer.isMulticast() && (first.type == ST_DATA_WITH_QOS || first.type == ST_QOS_NULL)
                && (!first.eosp || first.rspi)) {
            auto state = mib->findMeshPeerPowerState(first.peer);
            bool wantsOwner = !first.eosp && state && state->remote != MeshPowerMode::ACTIVE
                && !findPeriod(first.peer, MeshPeriodDirection::OWNER);
            bool wantsRecipient = first.rspi && state && state->local != MeshPowerMode::ACTIVE
                && !findPeriod(first.peer, MeshPeriodDirection::RECIPIENT);
            if (wantsOwner || wantsRecipient) {
                auto& trigger = triggers[first.peer];
                trigger.instance = instance(first.peer);
                trigger.failed = false;
                trigger.start = start;
                trigger.openingId = first.id;
                trigger.rspi = first.rspi;
                trigger.eosp = first.eosp;
                trigger.attempted.clear();
                for (const auto& member : members) trigger.attempted.insert(member.id);
            }
        }
    }
    for (const auto& member : members) {
        if (member.peer.isMulticast()) continue;
        if (auto owner = findPeriod(member.peer, MeshPeriodDirection::OWNER)) {
            if (member.id == owner->endingNull)
                owner->cap = std::min(owner->cap, member.ordinaryLimit);
            if (owner->ending && member.eosp && owner->cohort.count(member.id)) {
                if (owner->started.count(member.id)) owner->spent++;
                owner->started.insert(member.id);
            }
        }
    }
    // No notification between atomic debit and the existing immediate Tx handoff.
    return true;
}

void MeshPowerSave::prepareRecipientResponse(Packet *response, const Packet *eliciting,
        const Ptr<const Ieee80211MacHeader>& header, const std::optional<MeshPowerBaWindow>& window)
{
    Enter_Method("prepareRecipientResponse");
    if (!isEnabled()) return;
    auto peer = response->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress();
    auto period = findPeriod(peer, MeshPeriodDirection::RECIPIENT);
    if (!period || !mib->isMeshPeerEstablished(peer)) return;
    auto data = dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
    auto ba = dynamicPtrCast<const Ieee80211CompressedBlockAck>(response->peekAtFront<Ieee80211MacHeader>());
    int64_t receipt = -1;
    bool qualified = false;
    if (ba && window) {
        Tid tid = ba->getTidInfo();
        for (auto it = period->pendingEosp.begin(); it != period->pendingEosp.end();) {
            auto sequence = SequenceNumberCyclic(it->second);
            if (it->first == tid && (sequence < window->start || !(sequence < window->start+window->size)))
                it = period->pendingEosp.erase(it);
            else ++it;
        }
        for (const auto& ending : period->pendingEosp) {
            if (ending.first != tid) continue;
            int offset = OriginatorBlockAckAgreement::computeSequenceDistance(ba->getStartingSequenceNumber(), SequenceNumberCyclic(ending.second));
            qualified |= offset < 64 && ba->getBlockAckBitmap().getBit(offset);
        }
    }
    else if (data && data->getEosp()) receipt = eliciting->getId();
    if (!qualified && receipt == -1) return;
    auto stamp = response->addTagIfAbsent<Ieee80211MeshPowerResponseTag>();
    stamp->setPeer(peer); stamp->setLocalLinkId(period->instance.first); stamp->setPeerLinkId(period->instance.second);
    stamp->setOpeningId(period->openingId); stamp->setOpeningTime(period->openingTime);
    stamp->setReceiptId(receipt); stamp->setQualifyingBlockAck(qualified);
}

void MeshPowerSave::recipientResponseComplete(const Packet *response)
{
    Enter_Method("recipientResponseComplete");
    auto stamp = response->findTag<Ieee80211MeshPowerResponseTag>();
    if (!isEnabled() || !stamp || !mib->isMeshPeerEstablished(stamp->getPeer())) return;
    auto period = findPeriod(stamp->getPeer(), MeshPeriodDirection::RECIPIENT);
    if (!period || period->instance != instance(stamp->getPeer())
            || period->instance != MeshPeerInstance(stamp->getLocalLinkId(), stamp->getPeerLinkId())
            || period->openingId != stamp->getOpeningId() || period->openingTime != stamp->getOpeningTime()) return;
    if (stamp->getQualifyingBlockAck() || (period->ending && period->pendingReceipt == stamp->getReceiptId()))
        retirePeriod(stamp->getPeer(), MeshPeriodDirection::RECIPIENT, MeshPeriodOutcome::CONFIRMED);
}

void MeshPowerSave::recipientAgreementRetired(MacAddress peer, Tid tid)
{
    Enter_Method("recipientAgreementRetired");
    if (auto period = findPeriod(peer, MeshPeriodDirection::RECIPIENT)) {
        for (auto it = period->pendingEosp.begin(); it != period->pendingEosp.end();) {
            if (it->first == tid) it = period->pendingEosp.erase(it); else ++it;
        }
    }
}

void MeshPowerSave::queueServiceControl(MacAddress peer, bool rspi, bool eosp, bool ending)
{
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    auto localMode = mib->findMeshPeerPowerState(peer)->local;
    auto packet = std::unique_ptr<Packet>(makeModeIndication(peer, localMode));
    packet->setName(ending ? "mesh-period-ending" : "mesh-period-trigger");
    auto header = packet->removeAtFront<Ieee80211MeshDataHeader>();
    header->setRspi(rspi); header->setEosp(eosp);
    packet->insertAtFront(header);
    auto id = packet->getId();
    auto links = instance(peer);
    if (ending) {
        auto period = findPeriod(peer, MeshPeriodDirection::OWNER);
        if (!period || period->endingNull != -1) return;
        period->endingNull = id;
        if (period->ending) {
            period->replacementCreated = true;
            period->cohort.insert(id);
        }
    }
    else {
        Trigger intent;
        intent.instance = links;
        intent.queuedId = id;
        intent.rspi = rspi; intent.eosp = eosp;
        triggers[peer] = intent;
    }
    // Register before this consuming call: drop-NEW and listeners are synchronous.
    mac->sendMeshData(packet.release());
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer) || instance(peer) != links) return;
    if (ending) {
        auto period = findPeriod(peer, MeshPeriodDirection::OWNER);
        if (!period || period->endingNull != id) return;
    }
    else {
        auto intent = triggers.find(peer);
        if (intent == triggers.end() || intent->second.queuedId != id || intent->second.failed) return;
    }
    buffers->meshPowerEligibilityChanged();
}

void MeshPowerSave::reviewServicePeriods()
{
    Enter_Method("reviewServicePeriods");
    if (!isEnabled()) return;
    std::vector<MacAddress> peers;
    for (const auto& entry : mib->getMeshPeerPowerStates()) peers.push_back(entry.first);
    for (auto peer : peers) {
        if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) continue;
        auto inventory = buffers->getMeshBufferedFrames();
        bool payload = false;
        for (const auto& frame : inventory)
            payload |= frame.receiver == peer && frame.deadline > simTime()
                && (frame.kind == MeshBufferedFrame::Kind::PAYLOAD || frame.kind == MeshBufferedFrame::Kind::MANAGEMENT);
        auto owner = findPeriod(peer, MeshPeriodDirection::OWNER);
        if (owner) {
            if (!owner->ending && !payload && owner->endingNull == -1)
                queueServiceControl(peer, false, true, true);
            continue;
        }
        if (triggers.count(peer) || !canInitiateTo(peer, simTime())) continue;
        auto state = mib->findMeshPeerPowerState(peer);
        bool own = payload && state->remote != MeshPowerMode::ACTIVE;
        bool receive = peerWake[peer].retrievalRequested && state->local != MeshPowerMode::ACTIVE
            && !findPeriod(peer, MeshPeriodDirection::RECIPIENT);
        if (own || receive) queueServiceControl(peer, receive, !own, false);
    }
}

void MeshPowerSave::transmissionResolved(const MeshPowerResolution& result)
{
    Enter_Method("transmissionResolved");
    if (!isEnabled() || !mib->isMeshPeerEstablished(result.peer)) return;
    auto intent = triggers.find(result.peer);
    if (intent != triggers.end()) {
        bool retiredCandidate = false, retiredQueuedControl = false;
        for (const auto& retired : result.retired) {
            retiredQueuedControl |= intent->second.queuedId == retired.first;
            retiredCandidate |= intent->second.attempted.erase(retired.first) != 0;
        }
        // A partial DATA-trigger retirement cannot discard another actually
        // started member's later ACK/BAR success. Unknown recovery is not a
        // negative fact. With all candidates authoritatively gone, no response
        // can confirm this intent, even if an unrelated recovery remains.
        if (retiredQueuedControl || (retiredCandidate && intent->second.attempted.empty())) {
            intent->second.failed = true;
        }
    }
    auto period = findPeriod(result.peer, MeshPeriodDirection::OWNER);
    if (!period || period->instance != instance(result.peer)) return;
    bool relevant = false;
    for (auto id : result.originals) relevant |= period->cohort.count(id) || period->endingNull == id;
    for (const auto& retired : result.retired) relevant |= period->cohort.count(retired.first) || period->endingNull == retired.first;
    if (!relevant) return;
    if (result.barPending) period->barPending = *result.barPending;
    for (const auto& retired : result.retired)
        if (period->cohort.count(retired.first) || period->endingNull == retired.first)
            period->retired[retired.first] = retired.second;
    bool unresolved = false, ordinary = false, expired = false, disposed = false, live = false;
    for (const auto& id : period->cohort) {
        auto retired = period->retired.find(id);
        if (retired == period->retired.end()) { live = true; continue; }
        if (!retired->second) { unresolved = true; continue; }
        ordinary |= *retired->second == MeshPowerCause::ORDINARY_LIMIT;
        expired |= *retired->second == MeshPowerCause::EXPIRED;
        disposed |= *retired->second == MeshPowerCause::WITHDRAWN || *retired->second == MeshPowerCause::AGREEMENT_DISPOSAL;
    }
    auto endingControl = period->retired.find(period->endingNull);
    bool controlGone = endingControl != period->retired.end() && endingControl->second.has_value();
    if (unresolved) return;
    // A cause-only batch cannot erase a survivor's outstanding BAR proof.
    // Once every covered original is authoritatively retired, that exact
    // cohort has no remaining possible acknowledgment, irrespective of other
    // peer/TID recovery work. This also releases all-expired fallback.
    if (period->barPending && live) return;
    if (!live) period->barPending = false;
    if (ordinary || controlGone || (!live && disposed)) {
        retirePeriod(result.peer, MeshPeriodDirection::OWNER, MeshPeriodOutcome::FAILED);
        return;
    }
    if (period->ending && !live && expired && !period->replacementCreated && period->endingNull == -1) {
        queueServiceControl(result.peer, false, true, true);
        return;
    }
    if (period->ending && period->spent >= period->cap) {
        retirePeriod(result.peer, MeshPeriodDirection::OWNER, MeshPeriodOutcome::FAILED);
        return;
    }
    // Never-air initial ending control may be dropped before a cohort exists.
    if (!period->ending && controlGone)
        retirePeriod(result.peer, MeshPeriodDirection::OWNER, MeshPeriodOutcome::FAILED);
    radioControl->requestRadioReview();
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

void MeshPowerSave::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (strcmp(node.getTagName(), "set-mesh-power-mode"))
        throw cRuntimeError("Unsupported mesh power command '%s'", node.getTagName());
    const char *peerText = xmlutils::getMandatoryFilledAttribute(node, "peer");
    const char *modeText = xmlutils::getMandatoryFilledAttribute(node, "mode");
    MacAddress peer;
    if (!peer.tryParse(peerText) || peer.isUnspecified() || peer.isMulticast())
        throw cRuntimeError("Invalid individual mesh peer address '%s'", peerText);
    MeshPowerMode mode;
    if (!strcmp(modeText, "active")) mode = MeshPowerMode::ACTIVE;
    else if (!strcmp(modeText, "light-sleep")) mode = MeshPowerMode::LIGHT_SLEEP;
    else if (!strcmp(modeText, "deep-sleep")) mode = MeshPowerMode::DEEP_SLEEP;
    else throw cRuntimeError("Unknown mesh power mode '%s'", modeText);
    if (!isEnabled())
        throw cRuntimeError("Mesh power policy is disabled or not operating");
    if (!mib->isMeshPeerEstablished(peer))
        throw cRuntimeError("Mesh peer %s is not established", peer.str().c_str());
    requestMode(peer, mode);
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
    auto withdrawn = mode == MeshPowerMode::ACTIVE ? detachRecipient(peer) : std::optional<Period>();
    // A higher activity level takes effect immediately, including supersession.
    if (int(mode) < int(committed))
        mib->commitMeshLocalPowerMode(peer, mode);
    if (withdrawn) {
        PeriodOutcome fact(peer, MeshPeriodDirection::RECIPIENT, MeshPeriodOutcome::WITHDRAWN,
                withdrawn->openingId, withdrawn->openingTime);
        emit(meshServicePeriodOutcomeSignal, &fact);
        radioControl->requestRadioReview();
    }
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
    std::vector<PeriodOutcome> withdrawn;
    for (const auto& peer : peers)
        if (auto period = detachRecipient(peer.first))
            withdrawn.emplace_back(peer.first, MeshPeriodDirection::RECIPIENT, MeshPeriodOutcome::WITHDRAWN,
                    period->openingId, period->openingTime);
    mib->commitMeshLocalActiveForAllPeers();
    for (auto& fact : withdrawn)
        emit(meshServicePeriodOutcomeSignal, &fact);
    if (!withdrawn.empty()) radioControl->requestRadioReview();
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
    if (!isEnabled() || !mib->isMeshPeerEstablished(peer)) return;
    auto links = instance(peer);
    std::optional<MeshPowerMode> commitMode;
    auto pending = transitions.find(peer);
    if (pending != transitions.end() && pending->second.originalId == packet->getId() && pending->second.instance == links) {
        commitMode = pending->second.mode;
        transitions.erase(pending);
    }
    auto trigger = triggers.find(peer);
    if (trigger != triggers.end() && trigger->second.instance == links && trigger->second.attempted.count(packet->getId())) {
        auto completed = trigger->second;
        triggers.erase(trigger);
        if (!completed.eosp) openPeriod(peer, MeshPeriodDirection::OWNER, completed.openingId, completed.start);
        if (completed.rspi) openPeriod(peer, MeshPeriodDirection::RECIPIENT, completed.openingId, completed.start);
    }
    std::optional<PeriodOutcome> outcome;
    if (auto owner = findPeriod(peer, MeshPeriodDirection::OWNER)) {
        auto data = dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
        if (data && data->getEosp() && owner->cohort.count(packet->getId()) && owner->started.count(packet->getId())) {
            outcome.emplace(peer, MeshPeriodDirection::OWNER, MeshPeriodOutcome::CONFIRMED, owner->openingId, owner->openingTime);
            auto it = periods.find(peer);
            it->second[0].reset();
            if (!it->second[1]) periods.erase(it);
        }
    }
    // All local acknowledgment effects precede either synchronous publication.
    if (commitMode) mib->commitMeshLocalPowerMode(peer, *commitMode);
    if (outcome) emit(meshServicePeriodOutcomeSignal, &*outcome);
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
    auto id = packet->getId();
    for (auto& entry : periods)
        for (auto& period : entry.second)
            if (period) {
                if (period->cohort.count(id) || period->endingNull == id) period->retired.emplace(id, std::nullopt);
                else period->started.erase(id);
            }
    for (auto& entry : triggers)
        if (entry.second.queuedId == id) {
            entry.second.failed = true;
            entry.second.attempted.clear();
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
    std::set<int64_t> controls;
    auto trigger = triggers.find(peer);
    if (trigger != triggers.end() && trigger->second.queuedId != -1) controls.insert(trigger->second.queuedId);
    auto current = periods.find(peer);
    if (current != periods.end())
        for (const auto& period : current->second)
            if (period && period->endingNull != -1) controls.insert(period->endingNull);
    triggers.erase(peer);
    periods.erase(peer);
    radioControl->requestRadioReview();
    if (id != -1)
        buffers->withdrawMeshPowerControl(id);
    for (auto control : controls)
        if (control != id) buffers->withdrawMeshPowerControl(control);
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
        return findPeriod(bar->getReceiverAddress(), MeshPeriodDirection::OWNER) || !state || state->remote == MeshPowerMode::ACTIVE
            || (opportunity != awakeUntil.end() && start < opportunity->second);
    }
    auto type = header->getType();
    if (type == ST_BEACON || type == ST_PROBERESPONSE)
        return true;
    if (type == ST_QOS_NULL && header->getReceiverAddress().isMulticast())
        return true; // A group mode indication is not buffered group data.
    if (header->getReceiverAddress().isMulticast())
        return !hasSleepingPeer() || deliveringGroups;
    if (deliveringGroups && hasBufferedGroup({}))
        return false;
    auto state = mib->findMeshPeerPowerState(header->getReceiverAddress());
    auto owner = findPeriod(header->getReceiverAddress(), MeshPeriodDirection::OWNER);
    if (state) {
        bool wantsOwner = type == ST_DATA_WITH_QOS && state->remote != MeshPowerMode::ACTIVE && !owner;
        if (auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(header)) {
            if (type == ST_QOS_NULL) {
                wantsOwner = !mesh->getEosp() && state->remote != MeshPowerMode::ACTIVE && !owner;
                bool wantsRecipient = mesh->getRspi() && state->local != MeshPowerMode::ACTIVE
                    && !findPeriod(header->getReceiverAddress(), MeshPeriodDirection::RECIPIENT);
                if (wantsRecipient && !canInitiateTo(header->getReceiverAddress(), start)) return false;
            }
        }
        if (wantsOwner && !canInitiateTo(header->getReceiverAddress(), start)) return false;
    }
    if (owner && owner->ending && !owner->cohort.count(packet->getId()) && packet->getId() != owner->endingNull)
        return false;
    auto trigger = triggers.find(header->getReceiverAddress());
    if (type == ST_DATA_WITH_QOS && state && state->remote != MeshPowerMode::ACTIVE && !owner
            && trigger != triggers.end() && trigger->second.failed)
        return false;
    if (!owner && trigger != triggers.end() && !trigger->second.failed && trigger->second.queuedId != -1
            && packet->getId() != trigger->second.queuedId)
        return false;
    if (type == ST_QOS_NULL) return peerAwake(header->getReceiverAddress(), start);
    if (owner) return true;
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
        if (!peer.isMulticast() && mesh->getType() == ST_DATA_WITH_QOS) {
            auto owner = findPeriod(peer, MeshPeriodDirection::OWNER);
            mesh->setRspi(!owner && peerWake[peer].retrievalRequested);
            // The initial DATA trigger starts a period, even if it drains this
            // selection. OPEN-empty then ends with its first separate Null.
            mesh->setEosp(owner ? owner->ending : false);
            if (owner && !owner->ending) {
                bool remaining = false;
                for (const auto& frame : inventory)
                    remaining |= frame.receiver == peer && frame.deadline > simTime() && !deliveryUnit.count(frame.id)
                        && (frame.kind == MeshBufferedFrame::Kind::PAYLOAD || frame.kind == MeshBufferedFrame::Kind::MANAGEMENT);
                mesh->setEosp(!remaining);
            }
        }
        else if (peer.isMulticast()) mesh->setRspi(false);
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
    wake.retrievalRequested = individual;
    auto trigger = triggers.find(peer);
    if (trigger != triggers.end() && trigger->second.failed) triggers.erase(trigger);
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
    for (const auto& entry : periods) awake |= bool(entry.second[0]) || bool(entry.second[1]);
    for (const auto& entry : triggers) awake |= !entry.second.failed;
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
        if (entry.second.local == MeshPowerMode::ACTIVE) wake.retrievalRequested = false;
        bool required = entry.second.local == MeshPowerMode::LIGHT_SLEEP
            || (maintenanceDue && entry.first == peers.begin()->first);
        if (entry.second.local == MeshPowerMode::DEEP_SLEEP && entry.first == peers.begin()->first
                && !management->getPeerTbtt(entry.first, now + tick))
            awake = true; // Keep reacquiring an invalid selected deep reference.
        if (required) {
            auto tbtt = management->getPeerTbtt(entry.first, now + tick);
            if (!tbtt) awake = true; // Reacquire an absent/invalid reference while awake.
            else if (now >= *tbtt - lead) {
                if (!wake.rendezvous) {
                    wake.retrievalRequested |= wake.individualBuffered;
                    auto trigger = triggers.find(entry.first);
                    if (trigger != triggers.end() && trigger->second.failed) triggers.erase(trigger);
                }
                wake.rendezvous = true;
            }
            else boundary(*tbtt - lead);
        }
        awake |= wake.rendezvous || wake.retrievalRequested;
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
