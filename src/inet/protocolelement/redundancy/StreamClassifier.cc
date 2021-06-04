//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/redundancy/StreamClassifier.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamClassifier);

void StreamClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultGateIndex = par("defaultGateIndex");
        cStringTokenizer tokenizer(par("streamsToGateIndices"));
        while (tokenizer.hasMoreTokens()) {
            auto stream = tokenizer.nextToken();
            auto index = tokenizer.nextToken();
            streamsToGateIndexMap[stream] = atoi(index);
        }
    }
}

int StreamClassifier::classifyPacket(Packet *packet)
{
    auto streamReq = packet->getTag<StreamReq>();
    auto it = streamsToGateIndexMap.find(streamReq->getStreamName());
    if (it != streamsToGateIndexMap.end())
        return it->second;
    else
        return defaultGateIndex;
}

} // namespace inet

