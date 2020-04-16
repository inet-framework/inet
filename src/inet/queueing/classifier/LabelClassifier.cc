//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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

