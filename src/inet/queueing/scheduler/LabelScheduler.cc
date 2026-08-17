//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/LabelScheduler.h"

#include "inet/common/LabelsTag_m.h"

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
            collections.push_back(dynamic_cast<IPacketCollection *>(provider.get()));
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
    int originalIndex = index;
    for (auto collection : collections) {
        auto numPackets = collection->getNumPackets();
        if (index < numPackets)
            return collection->getPacket(index);
        index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", originalIndex);
}

void LabelScheduler::removePacket(Packet *packet)
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

int LabelScheduler::findInput(const PacketPredicate& predicate) const
{
    std::vector<Packet *> candidates;
    for (auto collection : collections)
        candidates.push_back(check_and_cast<IPacketExtractor *>(collection)->findPacket(predicate));
    for (auto label : labels) {
        for (size_t i = 0; i < candidates.size(); i++) {
            auto packet = candidates[i];
            if (packet == nullptr)
                continue;
            const auto& labelsTag = packet->findTag<LabelsTag>();
            if (labelsTag != nullptr) {
                for (size_t j = 0; j < labelsTag->getLabelsArraySize(); j++)
                    if (label == labelsTag->getLabels(j))
                        return i;
            }
        }
    }
    return defaultGateIndex >= 0 && defaultGateIndex < (int)candidates.size() && candidates[defaultGateIndex] != nullptr ? defaultGateIndex : -1;
}

Packet *LabelScheduler::findPacket(const PacketPredicate& predicate) const
{
    auto index = findInput(predicate);
    return index == -1 ? nullptr : check_and_cast<IPacketExtractor *>(collections[index])->findPacket(predicate);
}

Packet *LabelScheduler::dequeuePacket(const PacketPredicate& predicate)
{
    Enter_Method("dequeuePacket");
    auto index = findInput(predicate);
    if (index == -1)
        return nullptr;
    auto packet = check_and_cast<IPacketExtractor *>(collections[index])->dequeuePacket(predicate);
    ASSERT(packet != nullptr);
    take(packet);
    handlePacketProcessed(packet);
    emit(packetPulledSignal, packet);
    drop(packet);
    return packet;
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
        for (size_t i = 0; i < providers.size(); i++) {
            auto packet = providers[i].canPullPacket();
            const auto& labelsTag = packet->findTag<LabelsTag>();
            if (labelsTag != nullptr) {
                for (size_t j = 0; j < labelsTag->getLabelsArraySize(); j++)
                    if (label == labelsTag->getLabels(j))
                        return i;
            }
        }
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet
