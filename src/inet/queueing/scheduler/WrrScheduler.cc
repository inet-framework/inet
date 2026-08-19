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

        for (size_t i = 0; i < providers.size(); i++) {
            auto provider = providers[i].get();
            auto collection = dynamic_cast<IPacketCollection *>(provider);
            auto packetExtractor = dynamic_cast<IPacketExtractor *>(provider);
            collections.push_back(collection);
            packetExtractors.push_back(packetExtractor);
        }
    }
}

int WrrScheduler::getNumPackets() const
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

b WrrScheduler::getTotalLength() const
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

Packet *WrrScheduler::getPacket(int index) const
{
    int originalIndex = index;
    for (size_t i = 0; i < collections.size(); i++)
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot getPacket(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        auto numPackets = collection->getNumPackets();
        if (index < numPackets)
            return collection->getPacket(index);
        index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", originalIndex);
}

void WrrScheduler::removePacket(Packet *packet)
{
    for (size_t i = 0; i < collections.size(); i++)
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot removePacket(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    for (size_t i = 0; i < collections.size(); i++) {
        auto collection = collections[i];
        for (int i = 0; i < collection->getNumPackets(); i++) {
            if (collection->getPacket(i) == packet) {
                collection->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

int WrrScheduler::findInput(const PacketPredicate& predicate) const
{
    int firstWeighted = -1;
    int firstNonWeighted = -1;
    for (size_t i = 0; i < collections.size(); ++i) {
        if (packetExtractors[i] == nullptr)
            throw cRuntimeError("Cannot findPacket(): input provider at gate index %d does not implement IPacketExtractor", (int)i);
        if (packetExtractors[i]->findPacket(predicate) != nullptr) {
            if (buckets[i] > 0)
                return i;
            else if (firstWeighted == -1 && weights[i] > 0)
                firstWeighted = i;
            else if (firstNonWeighted == -1 && weights[i] == 0)
                firstNonWeighted = i;
        }
    }
    return firstWeighted != -1 ? firstWeighted : firstNonWeighted;
}

void WrrScheduler::consumeBucket(int index)
{
    if (weights[index] == 0)
        return;
    if (buckets[index] == 0) {
        for (size_t i = 0; i < collections.size(); ++i)
            buckets[i] = weights[i];
    }
    ASSERT(buckets[index] > 0);
    buckets[index]--;
}

Packet *WrrScheduler::findPacket(const PacketPredicate& predicate) const
{
    for (size_t i = 0; i < packetExtractors.size(); i++)
        if (packetExtractors[i] == nullptr)
            throw cRuntimeError("Cannot findPacket(): input provider at gate index %d does not implement IPacketExtractor", (int)i);
    auto index = findInput(predicate);
    return index == -1 ? nullptr : packetExtractors[index]->findPacket(predicate);
}

Packet *WrrScheduler::dequeuePacket(const PacketPredicate& predicate)
{
    Enter_Method("dequeuePacket");
    for (size_t i = 0; i < packetExtractors.size(); i++)
        if (packetExtractors[i] == nullptr)
            throw cRuntimeError("Cannot dequeuePacket(): input provider at gate index %d does not implement IPacketExtractor", (int)i);
    auto index = findInput(predicate);
    if (index == -1)
        return nullptr;
    auto packet = packetExtractors[index]->dequeuePacket(predicate);
    ASSERT(packet != nullptr);
    consumeBucket(index);
    take(packet);
    handlePacketProcessed(packet);
    emit(packetPulledSignal, packet);
    drop(packet);
    return packet;
}

void WrrScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    for (size_t i = 0; i < collections.size(); i++) {
        if (collections[i] == nullptr)
            throw cRuntimeError("Cannot removeAllPackets(): input provider at gate index %d does not implement IPacketCollection", (int)i);
    }
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
