//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/LabelClassifier.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(LabelClassifier);

void LabelClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultGateIndex = par("defaultGateIndex");
        cStringTokenizer tokenizer(par("labelsToGateIndices"));
        while (tokenizer.hasMoreTokens()) {
            auto label = tokenizer.nextToken();
            auto index = tokenizer.nextToken();
            labelsToGateIndexMap[label] = atoi(index);
        }
    }
}

int LabelClassifier::classifyPacket(Packet *packet)
{
    auto labelsTag = packet->getTag<LabelsTag>();
    for (int i = 0; i < (int)labelsTag->getLabelsArraySize(); i++) {
        auto label = labelsTag->getLabels(i);
        auto it = labelsToGateIndexMap.find(label);
        if (it != labelsToGateIndexMap.end())
            return it->second;
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet

