// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/linklayer/ieee80211/mesh/MeshPowerSave.h"

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"

namespace inet::ieee80211 {

Define_Module(MeshPowerSave);

void MeshPowerSave::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        enabled = par("enabled");
        mib.reference(this, "mibModule", true);
        management.reference(this, "managementModule", true);
        buffers.reference(this, "hcfModule", true);
        mac.reference(this, "macModule", true);
        WATCH(deliveringGroups);
        WATCH_MAP(transitions);
        WATCH_MAP(awakeUntil);
    }
}

void MeshPowerSave::handleMessageWhenUp(cMessage *)
{
    throw cRuntimeError("MeshPowerSave uses typed policy calls; it owns no wake scheduler");
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
}

void MeshPowerSave::clearPolicy()
{
    auto old = transitions;
    transitions.clear();
    awakeUntil.clear();
    deliveringGroups = false;
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
}

void MeshPowerSave::peerRemoved(MacAddress peer)
{
    Enter_Method("peerRemoved");
    auto pending = transitions.find(peer);
    int64_t id = pending == transitions.end() ? -1 : pending->second.originalId;
    transitions.erase(peer);
    awakeUntil.erase(peer);
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
    if (!header) return true; // Required ACK/BA/BAR responses keep their MAC owner.
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
                || frame.kind == MeshBufferedFrame::Kind::POWER_CONTROL)
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
}

void MeshPowerSave::groupTransmissionComplete(const Packet *packet)
{
    Enter_Method("groupTransmissionComplete");
    auto current = transitions.find(MacAddress::BROADCAST_ADDRESS);
    if (current != transitions.end() && current->second.originalId == packet->getId())
        transitions.erase(current);
    if (deliveringGroups && !hasBufferedGroup({packet->getId()})) deliveringGroups = false;
}

} // namespace inet::ieee80211
