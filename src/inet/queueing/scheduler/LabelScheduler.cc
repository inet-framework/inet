//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/LabelScheduler.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(LabelScheduler);

void LabelScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultGateIndex = par("defaultGateIndex");
        labels = cStringTokenizer(par("labels")).asVector();
        for (auto provider : providers)
            collections.push_back(dynamic_cast<IPacketCollection *>(provider));
    }
}

int LabelScheduler::getNumPackets() const
{
    int size = 0;
    for (auto collection : collections)
        size += collection->getNumPackets();
    return size;
}

b LabelScheduler::getTotalLength() const
{
    b totalLength(0);
    for (auto collection : collections)
        totalLength += collection->getTotalLength();
    return totalLength;
}

Packet *LabelScheduler::getPacket(int index) const
{
    throw cRuntimeError("TODO");
}

void LabelScheduler::removePacket(Packet *packet)
{
    throw cRuntimeError("TODO");
}

void LabelScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    for (auto collection : collections)
        collection->removeAllPackets();
}

int LabelScheduler::schedulePacket()
{
    for (auto label : labels) {
        for (int i = 0; i < (int)providers.size(); i++) {
            auto packet = providers[i]->canPullPacket(inputGates[i]->getPathStartGate());
            const auto& labelsTag = packet->findTag<LabelsTag>();
            if (labelsTag != nullptr) {
                for (int j = 0; j < (int)labelsTag->getLabelsArraySize(); j++)
                    if (label == labelsTag->getLabels(j))
                        return i;
            }
        }
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet

