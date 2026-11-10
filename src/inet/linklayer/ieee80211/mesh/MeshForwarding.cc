// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mesh/MeshForwarding.h"

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
        mib->setMeshForwarding(par("forwarding"));
        if (initialTtl < 1 || initialTtl > 255 || cacheCapacity < 1 || cacheLifetime <= SIMTIME_ZERO)
            throw cRuntimeError("Invalid mesh TTL or duplicate-cache policy");
        WATCH(nextSequence);
        WATCH_EXPR("duplicateCacheSize", recentFrames.size());
    }
}

void MeshForwarding::handleMessageWhenUp(cMessage *) { throw cRuntimeError("Mesh forwarding uses direct frame contracts"); }
void MeshForwarding::handleStartOperation(LifecycleOperation *) { recentFrames.clear(); nextSequence = 0; }
void MeshForwarding::handleStopOperation(LifecycleOperation *) { recentFrames.clear(); }
void MeshForwarding::handleCrashOperation(LifecycleOperation *) { recentFrames.clear(); }

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
    auto path = destination.isMulticast() ? std::nullopt : findPath(destination);
    if (destination.isUnspecified() || (!destination.isMulticast() && !path)) { discard(packet); return; }
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
    // IEEE Std 802.11-2024, 10.37.3.2: even a final destination requires a live precursor.
    if (!group && (!pathSelection || (destination != mib->address && !path)
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
