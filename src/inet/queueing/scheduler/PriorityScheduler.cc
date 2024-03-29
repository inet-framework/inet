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
        for (auto provider : providers)
            collections.push_back(dynamic_cast<IPacketCollection *>(provider.get()));
    }
}

int PriorityScheduler::getNumPackets() const
{
    int size = 0;
    for (auto collection : collections)
        if (collection != nullptr)
            size += collection->getNumPackets();
        else
            return -1;
    return size;
}

b PriorityScheduler::getTotalLength() const
{
    b totalLength(0);
    for (auto collection : collections)
        if (collection != nullptr)
            totalLength += collection->getTotalLength();
        else
            return b(-1);
    return totalLength;
}

Packet *PriorityScheduler::getPacket(int index) const
{
    int origIndex = index;
    for (auto collection : collections) {
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
    for (auto collection : collections) {
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

void PriorityScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
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

