//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/LabelClassifier.h"

#include "inet/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(LabelClassifier);

void LabelClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultGateIndex = par("defaultGateIndex");
        auto labelsToGateIndices = check_and_cast<cValueMap *>(par("labelsToGateIndices").objectValue());
        for (auto elem : labelsToGateIndices->getFields()) {
            auto label = elem.first;
            auto index = elem.second.intValue();
            labelsToGateIndexMap[label] = index;
        }
    }
}

int LabelClassifier::classifyPacket(Packet *packet)
{
    auto labelsTag = packet->getTag<LabelsTag>();
    for (size_t i = 0; i < labelsTag->getLabelsArraySize(); i++) {
        auto label = labelsTag->getLabels(i);
        auto it = labelsToGateIndexMap.find(label);
        if (it != labelsToGateIndexMap.end())
            return it->second;
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet

