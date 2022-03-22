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
            collections.push_back(dynamic_cast<IPacketCollection *>(provider));
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
        if (providers[i]->canPullSomePacket(inputGates[i]->getPathStartGate())) {
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

