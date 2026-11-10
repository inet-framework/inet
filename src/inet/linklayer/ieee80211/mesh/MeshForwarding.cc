// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mesh/MeshForwarding.h"

#include <algorithm>

#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(MeshForwarding);

void MeshForwarding::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mac.reference(this, "macModule", true);
        mib.reference(this, "mibModule", true);
        pathSelection.reference(this, "pathSelectionModule", false);
        initialTtl = par("initialTtl");
        cacheCapacity = par("duplicateCacheCapacity");
        cacheLifetime = par("duplicateCacheLifetime");
        maxPendingPackets = par("maxPendingPackets");
        maxPendingBytes = B(par("maxPendingBytes").intValue());
        maxPendingWait = par("maxPendingWait");
        if (maxPendingPackets < 1 || maxPendingBytes <= b(0) || maxPendingWait <= SIMTIME_ZERO)
            throw cRuntimeError("Invalid pending mesh source queue policy");
        pendingExpiryTimer = new cMessage("meshPendingExpiry");
        pendingExpiryTimer->setSchedulingPriority(1);
        mib->setMeshForwarding(par("forwarding"));
        if (initialTtl < 1 || initialTtl > 255 || cacheCapacity < 1 || cacheLifetime <= SIMTIME_ZERO)
            throw cRuntimeError("Invalid mesh TTL or duplicate-cache policy");
        WATCH(nextSequence);
        WATCH_EXPR("duplicateCacheSize", recentFrames.size());
        WATCH_EXPR("pendingPackets", pending.size());
        WATCH(pendingBytes);
    }
}

void MeshForwarding::handleMessageWhenUp(cMessage *message)
{
    if (message != pendingExpiryTimer)
        throw cRuntimeError("Unknown mesh forwarding timer");
    expirePending();
}
void MeshForwarding::handleStartOperation(LifecycleOperation *) { recentFrames.clear(); }
void MeshForwarding::handleStopOperation(LifecycleOperation *) { clearPending(); recentFrames.clear(); }
void MeshForwarding::handleCrashOperation(LifecycleOperation *) { clearPending(); recentFrames.clear(); }

MeshForwarding::~MeshForwarding()
{
    cancelAndDelete(pendingExpiryTimer);
    for (const auto& entry : pending)
        delete entry.packet;
}

bool MeshForwarding::hasPending(MacAddress destination) const
{
    return std::any_of(pending.begin(),pending.end(),[&](const Pending& entry) { return entry.destination == destination; });
}

MeshForwarding::Pending MeshForwarding::removePending(std::list<Pending>::iterator it)
{
    auto entry = *it;
    pendingBytes -= entry.packet->getDataLength();
    pending.erase(it);
    return entry;
}

void MeshForwarding::scheduleExpiry()
{
    cancelEvent(pendingExpiryTimer);
    if (pending.empty() || operationalState != OPERATING)
        return;
    auto deadline = pending.front().expiry;
    for (const auto& entry : pending)
        deadline = std::min(deadline,entry.expiry);
    scheduleAt(deadline,pendingExpiryTimer);
}

void MeshForwarding::expirePending()
{
    while (true) {
        auto it = std::find_if(pending.begin(),pending.end(),[&](const Pending& entry) { return simTime() >= entry.expiry; });
        if (it == pending.end())
            break;
        auto entry = removePending(it);
        if (!hasPending(entry.destination) && pathSelection)
            pathSelection->cancelDiscovery(entry.destination);
        discard(entry.packet);
    }
    scheduleExpiry();
}

void MeshForwarding::clearPending()
{
    cancelEvent(pendingExpiryTimer);
    std::set<MacAddress> destinations;
    auto owned = std::move(pending);
    pending.clear();
    pendingBytes = b(0);
    for (const auto& entry : owned) {
        destinations.insert(entry.destination);
        delete entry.packet;
    }
    if (pathSelection)
        for (auto destination : destinations)
            pathSelection->cancelDiscovery(destination);
}

void MeshForwarding::pathAvailable(MacAddress destination)
{
    Enter_Method("pathAvailable");
    if (operationalState != OPERATING)
        return;
    expirePending();
    while (hasPending(destination)) {
        auto path = findPath(destination);
        if (!path)
            break;
        auto it = std::find_if(pending.begin(),pending.end(),[&](const Pending& entry) { return entry.destination == destination; });
        auto entry = removePending(it);
        if (!hasPending(destination))
            pathSelection->cancelDiscovery(destination);
        sendResolved(entry.packet,destination,path);
    }
    scheduleExpiry();
}

void MeshForwarding::discoveryFailed(MacAddress destination)
{
    Enter_Method("discoveryFailed");
    while (hasPending(destination)) {
        auto it = std::find_if(pending.begin(),pending.end(),[&](const Pending& entry) { return entry.destination == destination; });
        auto entry = removePending(it);
        discard(entry.packet);
    }
    scheduleExpiry();
}

bool MeshForwarding::hasPendingSource(MacAddress destination)
{
    Enter_Method("hasPendingSource");
    if (operationalState != OPERATING) return false;
    expirePending();
    return hasPending(destination);
}

void MeshForwarding::discard(Packet *packet)
{
    PacketDropDetails details;
    details.setReason(OTHER_PACKET_DROP);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
}

bool MeshForwarding::remember(MacAddress source, uint32_t sequence)
{
    // IEEE Std 802.11-2024, 10.37.6. Cache policy is local and deterministic.
    for (auto it = recentFrames.begin(); it != recentFrames.end(); )
        if (simTime() >= it->second)
            it = recentFrames.erase(it);
        else
            ++it;
    auto key = std::make_pair(source, sequence);
    if (recentFrames.count(key))
        return false;
    if (recentFrames.size() >= size_t(cacheCapacity)) {
        auto oldest = recentFrames.begin();
        for (auto it = recentFrames.begin(); it != recentFrames.end(); ++it)
            if (it->second < oldest->second)
                oldest = it;
        recentFrames.erase(oldest);
    }
    recentFrames.emplace(key, simTime() + cacheLifetime);
    return true;
}

std::optional<MeshPath> MeshForwarding::findPath(MacAddress destination) const
{
    auto path = pathSelection ? pathSelection->findPath(destination) : std::nullopt;
    if (!path || simTime() >= path->expiry || !mib->isMeshPeerEstablished(path->nextHop))
        return std::nullopt;
    return path;
}

void MeshForwarding::processSourceMsdu(Packet *packet)
{
    Enter_Method("processSourceMsdu");
    take(packet);
    if (operationalState != OPERATING) { discard(packet); return; }
    auto destination = packet->getTag<MacAddressReq>()->getDestAddress();
    if (destination.isUnspecified()) { discard(packet); return; }
    expirePending();
    if (!destination.isMulticast() && hasPending(destination))
        pathAvailable(destination);
    auto path = destination.isMulticast() ? std::nullopt : findPath(destination);
    if (!destination.isMulticast() && !path) {
        if (!pathSelection || pending.size() >= size_t(maxPendingPackets) || packet->getDataLength() > maxPendingBytes-pendingBytes) {
            discard(packet); return;
        }
        pending.push_back({packet,destination,simTime()+maxPendingWait});
        pendingBytes += packet->getDataLength();
        scheduleExpiry();
        pathSelection->startDiscovery(destination);
        return;
    }
    sendResolved(packet,destination,path);
}

void MeshForwarding::sendResolved(Packet *packet, MacAddress destination, const std::optional<MeshPath>& path)
{
    packet->addTagIfAbsent<LlcProtocolTag>()->setProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    auto header = makeShared<Ieee80211MeshDataHeader>();
    header->setType(ST_DATA_WITH_QOS);
    header->setFromDS(true);
    header->setToDS(!destination.isMulticast());
    header->setChunkLength(destination.isMulticast() ? B(26) : B(32));
    header->setReceiverAddress(destination.isMulticast() ? destination : path->nextHop);
    header->setTransmitterAddress(mib->address);
    header->setAddress3(destination.isMulticast() ? mib->address : destination);
    if (!destination.isMulticast())
        header->setAddress4(mib->address);
    header->setMeshControlPresent(true);
    if (auto priority = packet->findTag<UserPriorityReq>())
        header->setTid(priority->getUserPriority());
    auto control = makeShared<Ieee80211MeshControl>();
    control->setTtl(initialTtl);
    control->setSequenceNumber(nextSequence++); // modulo-2^32, independent of hop MAC sequencing
    if (destination.isMulticast())
        remember(mib->address, control->getSequenceNumber());
    packet->insertAtFront(control);
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    mac->sendMeshData(packet);
}

void MeshForwarding::deliver(Packet *packet, MacAddress source, MacAddress destination, int tid)
{
    packet->popAtFront<Ieee80211DataHeader>();
    packet->popAtFront<Ieee80211MeshControl>();
    packet->popAtBack<Ieee80211MacTrailer>(B(4));
    packet->trim();
    mac->deliverMeshMsdu(packet, source, destination, tid);
}

void MeshForwarding::processDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    Enter_Method("processDataFrame");
    take(packet);
    auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
    if (operationalState != OPERATING || !mesh || !mib->isMeshPeerEstablished(header->getTransmitterAddress())
            || header->getType() != ST_DATA_WITH_QOS || !mesh->getMeshControlPresent()
            || header->getMoreFragments() || header->getFragmentNumber() != 0 || header->getAMsduPresent()
            || !header->getFromDS() || header->getToDS() == header->getReceiverAddress().isMulticast()) {
        discard(packet); return;
    }
    auto remaining = packet->getDataLength() - header->getChunkLength() - B(4);
    if (remaining < B(6)) { discard(packet); return; }
    auto control = packet->peekDataAt<Ieee80211MeshControl>(header->getChunkLength(), b(-1),
            Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
    if (control->isIncorrect() || control->isIncomplete() || control->getAddressExtensionMode() != 0 || control->getTtl() == 0) { discard(packet); return; }
    bool group = header->getReceiverAddress().isMulticast();
    auto source = group ? header->getAddress3() : header->getAddress4();
    auto destination = group ? header->getReceiverAddress() : header->getAddress3();
    if (source.isUnspecified() || source.isMulticast() || destination.isUnspecified() || (!group && destination.isMulticast())) { discard(packet); return; }
    auto path = group || destination == mib->address ? std::nullopt : findPath(destination);
    if (!group && destination != mib->address && !path) {
        if (pathSelection)
            pathSelection->reportUnknownTransit(header->getTransmitterAddress(),destination);
        discard(packet); return;
    }
    // IEEE Std 802.11-2024, 10.37.3.2: even a final destination requires a live precursor.
    if (!group && (!pathSelection
            || simTime() >= pathSelection->getPrecursorExpiry(destination, header->getTransmitterAddress()))) {
        discard(packet); return;
    }
    if (!remember(source, control->getSequenceNumber())) { discard(packet); return; }
    if (!group) {
        pathSelection->refreshPath(source);
        pathSelection->refreshPrecursor(destination, header->getTransmitterAddress());
        if (destination == mib->address) {
            deliver(packet, source, destination, header->getTid());
            return; // final delivery does not decrement Mesh TTL
        }
        pathSelection->refreshPath(destination);
        pathSelection->refreshPrecursor(source, path->nextHop);
    }
    // 10.37.3.2/10.37.4.2: a nonzero received TTL decrements to zero;
    // group local delivery survives that exhausted forwarding budget.
    auto ttl = control->getTtl() - 1;
    if (ttl > 0 && mib->isMeshForwarding()) {
        auto forwarded = packet->dup();
        forwarded->trim(); // discard popped PHY history before the next hop computes FCS
        auto outgoing = forwarded->removeAtFront<Ieee80211MeshDataHeader>();
        auto outgoingControl = forwarded->removeAtFront<Ieee80211MeshControl>();
        outgoing->setTransmitterAddress(mib->address);
        outgoing->setRetry(false);
        if (!group)
            outgoing->setReceiverAddress(path->nextHop);
        outgoingControl->setTtl(ttl);
        forwarded->insertAtFront(outgoingControl);
        forwarded->insertAtFront(outgoing);
        mac->sendMeshData(forwarded);
    }
    if (group)
        deliver(packet, source, destination, header->getTid());
    else
        discard(packet);
}

} // namespace ieee80211
} // namespace inet
