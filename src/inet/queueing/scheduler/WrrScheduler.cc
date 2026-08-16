//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/WrrScheduler.h"

#include "inet/common/INETUtils.h"

namespace inet {
namespace queueing {

Define_Module(WrrScheduler);

WrrScheduler::~WrrScheduler()
{
    delete[] weights;
    delete[] buckets;
}

void WrrScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        weights = new unsigned int[providers.size()];
        buckets = new unsigned int[providers.size()];

        cStringTokenizer tokenizer(par("weights"));
        size_t i;
        for (i = 0; i < providers.size() && tokenizer.hasMoreTokens(); ++i)
            buckets[i] = weights[i] = utils::atoul(tokenizer.nextToken());

        if (i < providers.size())
            throw cRuntimeError("Too few values given in the weights parameter.");
        if (tokenizer.hasMoreTokens())
            throw cRuntimeError("Too many values given in the weights parameter.");

        for (auto provider : providers)
            collections.push_back(dynamic_cast<IPacketCollection *>(provider.get()));
    }
}

int WrrScheduler::getNumPackets() const
{
    int size = 0;
    for (auto collection : collections)
        size += collection->getNumPackets();
    return size;
}

b WrrScheduler::getTotalLength() const
{
    b totalLength(0);
    for (auto collection : collections)
        totalLength += collection->getTotalLength();
    return totalLength;
}

Packet *WrrScheduler::getPacket(int index) const
{
    int originalIndex = index;
    for (auto collection : collections) {
        auto numPackets = collection->getNumPackets();
        if (index < numPackets)
            return collection->getPacket(index);
        index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", originalIndex);
}

void WrrScheduler::removePacket(Packet *packet)
{
    for (auto collection : collections) {
        for (int i = 0; i < collection->getNumPackets(); i++) {
            if (collection->getPacket(i) == packet) {
                collection->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

Packet *WrrScheduler::dequeuePacket(Packet *packet)
{
    Enter_Method("dequeuePacket");
    for (auto collection : collections) {
        for (int i = 0; i < collection->getNumPackets(); i++) {
            if (collection->getPacket(i) == packet) {
                packet = check_and_cast<IPacketExtractor *>(collection)->dequeuePacket(packet);
                take(packet);
                handlePacketProcessed(packet);
                emit(packetPulledSignal, packet);
                drop(packet);
                return packet;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

void WrrScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    for (auto collection : collections)
        collection->removeAllPackets();
}

int WrrScheduler::schedulePacket()
{
    int firstWeighted = -1;
    int firstNonWeighted = -1;
    for (size_t i = 0; i < providers.size(); ++i) {
        if (providers[i].canPullSomePacket()) {
            if (buckets[i] > 0) {
                buckets[i]--;
                return (int)i;
            }
            else if (firstWeighted == -1 && weights[i] > 0)
                firstWeighted = (int)i;
            else if (firstNonWeighted == -1 && weights[i] == 0)
                firstNonWeighted = (int)i;
        }
    }

    if (firstWeighted != -1) {
        for (size_t i = 0; i < providers.size(); ++i)
            buckets[i] = weights[i];
        buckets[firstWeighted]--;
        return firstWeighted;
    }

    if (firstNonWeighted != -1)
        return firstNonWeighted;

    return -1;
}

} // namespace queueing
} // namespace inet
