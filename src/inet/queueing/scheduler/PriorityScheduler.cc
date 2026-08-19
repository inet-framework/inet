//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/PriorityScheduler.h"

namespace inet {
namespace queueing {

Define_Module(PriorityScheduler);

void PriorityScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (size_t i = 0; i < providers.size(); i++) {
            auto provider = providers[i].get();
            collections.push_back(dynamic_cast<IPacketCollection *>(provider));
            auto packetExtractor = dynamic_cast<IPacketExtractor *>(provider);
            packetExtractors.push_back(packetExtractor);
        }
    }
}

int PriorityScheduler::getNumPackets() const
{
    int size = 0;
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        if (collection == nullptr)
            throw cRuntimeError("Cannot getNumPackets(): input provider at gate index %d does not implement IPacketCollection", (int)i);
        size += collection->getNumPackets();
    }
    return size;
}

b PriorityScheduler::getTotalLength() const
{
    b totalLength(0);
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        if (collection == nullptr)
            throw cRuntimeError("Cannot getTotalLength(): input provider at gate index %d does not implement IPacketCollection", (int)i);
        totalLength += collection->getTotalLength();
    }
    return totalLength;
}

Packet *PriorityScheduler::getPacket(int index) const
{
    int origIndex = index;
    for (size_t i = 0; i < collections.size(); i++)
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot getPacket(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        auto numPackets = collection->getNumPackets();
        if (index < numPackets)
            return collection->getPacket(index);
        else
            index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", origIndex);
}

void PriorityScheduler::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    for (size_t i = 0; i < collections.size(); i++)
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot removePacket(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        int numPackets = collection->getNumPackets();
        for (int j = 0; j < numPackets; j++) {
            if (collection->getPacket(j) == packet) {
                collection->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

Packet *PriorityScheduler::findPacket(const PacketPredicate& predicate) const
{
    for (size_t i = 0; i < packetExtractors.size(); i++)
        if (packetExtractors[i] == nullptr)
            throw cRuntimeError("Cannot findPacket(): input provider at gate index %d does not implement IPacketExtractor", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto index = reverseOrder ? collections.size() - i - 1 : i;
        if (packetExtractors[index] == nullptr)
            throw cRuntimeError("Cannot findPacket(): input provider at gate index %d does not implement IPacketExtractor", (int)index);
        auto packet = packetExtractors[index]->findPacket(predicate);
        if (packet != nullptr)
            return packet;
    }
    return nullptr;
}

Packet *PriorityScheduler::dequeuePacket(const PacketPredicate& predicate)
{
    Enter_Method("dequeuePacket");
    for (size_t i = 0; i < packetExtractors.size(); i++)
        if (packetExtractors[i] == nullptr)
            throw cRuntimeError("Cannot dequeuePacket(): input provider at gate index %d does not implement IPacketExtractor", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto index = reverseOrder ? collections.size() - i - 1 : i;
        if (packetExtractors[index] == nullptr)
            throw cRuntimeError("Cannot dequeuePacket(): input provider at gate index %d does not implement IPacketExtractor", (int)index);
        auto packet = packetExtractors[index]->dequeuePacket(predicate);
        if (packet == nullptr)
            continue;
        take(packet);
        handlePacketProcessed(packet);
        emit(packetPulledSignal, packet);
        drop(packet);
        return packet;
    }
    return nullptr;
}

void PriorityScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    for (size_t i = 0; i < collections.size(); i++) {
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot removeAllPackets(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    }
    for (auto collection : collections)
        collection->removeAllPackets();
}

int PriorityScheduler::schedulePacket()
{
    for (size_t i = 0; i < providers.size(); i++) {
        int inputIndex = getInputGateIndex(i);
        if (inputIndex == inProgressGateIndex || providers[inputIndex].canPullSomePacket())
            return inputIndex;
    }
    return -1;
}

void PriorityScheduler::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (isStreamingPacket()) {
        EV_INFO << "Checking if the ongoing packet streaming should be ended" << EV_ENDL;
        int index = callSchedulePacket();
        if (index != inProgressGateIndex) {
            auto packet = providers[inProgressGateIndex].pullPacketEnd();
            EV_INFO << "Ending packet streaming" << EV_FIELD(packet) << EV_ENDL;
            take(packet);
            endPacketStreaming(packet);
            consumer.pushPacketEnd(packet);
        }
    }
    PacketSchedulerBase::handleCanPullPacketChanged(gate);
}

} // namespace queueing
} // namespace inet
