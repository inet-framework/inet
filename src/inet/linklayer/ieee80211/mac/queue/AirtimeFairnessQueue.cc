//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessQueue.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211AirtimeInd.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessQueue);

void AirtimeFairnessQueue::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetCapacity = par("packetCapacity");
        quantum = par("quantum");
        weight = par("weight");
        fairnessEnabled = par("fairnessEnabled");
        if (quantum <= SIMTIME_ZERO)
            throw cRuntimeError("The quantum parameter must be positive");
        if (weight <= 0)
            throw cRuntimeError("The weight parameter must be positive");
        frameTransmittedAirtimeSignal = registerSignal("frameTransmittedAirtime");
        WATCH(numPackets);
        WATCH_EXPR("numStations", (int)activeList.size()); // backlogged stations currently in the round-robin
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // The coordination function (Dcf/Hcf) emits frameTransmittedAirtime from within
        // the containing network interface; subscribing there scopes the accounting to
        // this interface's own transmissions (the frame's receiver disambiguates stations).
        getContainingNicModule(this)->subscribe(frameTransmittedAirtimeSignal, this);
    }
}

MacAddress AirtimeFairnessQueue::getReceiverAddress(Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    return header->getReceiverAddress();
}

AirtimeFairnessQueue::StationState& AirtimeFairnessQueue::getOrCreateStation(const MacAddress& address)
{
    auto it = stations.find(address);
    if (it == stations.end()) {
        auto& state = stations[address];
        state.weight = weight;
        return state;
    }
    return it->second;
}

void AirtimeFairnessQueue::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;
    packet->setArrivalTime(simTime());
    MacAddress address = getReceiverAddress(packet);
    auto& state = getOrCreateStation(address);
    bool wasBacklogged = !state.frames.empty();
    state.frames.push_back(packet);
    numPackets++;
    if (!wasBacklogged)
        activeList.push_back(address); // station became backlogged -> joins the round-robin
    // tail drop: keep the total number of stored packets within capacity
    while (packetCapacity != -1 && numPackets > packetCapacity) {
        // drop the just-pushed packet (drop-tail on the arriving station)
        Packet *droppedPacket = state.frames.back();
        removeStoredPacket(address, droppedPacket);
        EV_INFO << "Dropping packet" << EV_FIELD(droppedPacket) << EV_ENDL;
        dropPacket(droppedPacket, QUEUE_OVERFLOW, packetCapacity);
    }
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
}

Packet *AirtimeFairnessQueue::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    if (activeList.empty())
        throw cRuntimeError("Cannot pull packet from an empty queue");
    // classic deficit round-robin with sign-based eligibility (mac80211 style):
    // advance the cursor, topping up ineligible (negative-deficit) stations, until an
    // eligible station is found; guaranteed to terminate since quantum and weight are positive.
    while (true) {
        MacAddress address = activeList.front();
        auto& state = stations.at(address);
        if (!fairnessEnabled || state.deficit >= SIMTIME_ZERO) {
            Packet *packet = state.frames.front();
            state.frames.pop_front();
            numPackets--;
            activeList.pop_front();
            if (!state.frames.empty())
                activeList.push_back(address); // rotate: give the other stations a turn
            // else the station drained -> leaves the round-robin; its deficit is kept so a
            // still-pending airtime charge (or its later return) is accounted correctly.
            EV_INFO << "Pulling packet" << EV_FIELD(packet) << EV_ENDL;
            auto queueingTime = simTime() - packet->getArrivalTime();
            increaseTimeTag<QueueingTimeTag>(packet, queueingTime, queueingTime);
            emit(packetPulledSignal, packet);
            return packet;
        }
        else {
            state.deficit += quantum * state.weight;
            activeList.pop_front();
            activeList.push_back(address);
        }
    }
}

void AirtimeFairnessQueue::receiveSignal(cComponent *source, simsignal_t signalID, cObject *object, cObject *details)
{
    if (signalID == frameTransmittedAirtimeSignal) {
        Enter_Method("%s", cComponent::getSignalName(signalID));
        auto info = check_and_cast<Ieee80211AirtimeInd *>(object);
        auto it = stations.find(info->receiverAddress);
        // only charge stations this queue actually serves; frames sent on behalf of another
        // queue (e.g. a different access category) are not ours to account for
        if (it != stations.end()) {
            it->second.deficit -= info->airtime;
            EV_DEBUG << "Charged " << info->airtime << " airtime to " << info->receiverAddress
                     << ", deficit now " << it->second.deficit << EV_ENDL;
        }
    }
}

void AirtimeFairnessQueue::removeStoredPacket(const MacAddress& address, Packet *packet)
{
    auto& state = stations.at(address);
    auto& frames = state.frames;
    auto it = std::find(frames.begin(), frames.end(), packet);
    if (it == frames.end())
        throw cRuntimeError("Packet not found in station queue");
    frames.erase(it);
    numPackets--;
    if (frames.empty())
        activeList.remove(address); // no longer backlogged -> leaves the round-robin
}

b AirtimeFairnessQueue::getTotalLength() const
{
    b totalLength(0);
    for (auto& element : stations)
        for (auto packet : element.second.frames)
            totalLength += packet->getTotalLength();
    return totalLength;
}

Packet *AirtimeFairnessQueue::getPacket(int index) const
{
    if (index < 0 || index >= numPackets)
        throw cRuntimeError("index %i out of range", index);
    int i = index;
    for (auto& element : stations) {
        auto& frames = element.second.frames;
        if (i < (int)frames.size())
            return frames[i];
        i -= frames.size();
    }
    throw cRuntimeError("index %i out of range", index);
}

void AirtimeFairnessQueue::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    EV_INFO << "Removing packet" << EV_FIELD(packet) << EV_ENDL;
    MacAddress address = getReceiverAddress(packet);
    removeStoredPacket(address, packet);
    emit(packetRemovedSignal, packet);
}

void AirtimeFairnessQueue::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    EV_INFO << "Removing all packets" << EV_ENDL;
    std::vector<Packet *> packets;
    for (auto& element : stations)
        for (auto packet : element.second.frames)
            packets.push_back(packet);
    stations.clear();
    activeList.clear();
    numPackets = 0;
    for (auto packet : packets) {
        emit(packetRemovedSignal, packet);
        delete packet;
    }
}

} // namespace ieee80211
} // namespace inet
