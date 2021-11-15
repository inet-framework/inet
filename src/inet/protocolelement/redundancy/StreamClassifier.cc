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
        mode = par("mode");
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
        gateIndexOffset = par("gateIndexOffset");
        defaultGateIndex = par("defaultGateIndex");
    }
}

int StreamClassifier::classifyPacket(Packet *packet)
{
    const char *streamName = nullptr;
    switch (*mode) {
        case 'r': {
            auto streamReq = packet->findTag<StreamReq>();
            if (streamReq != nullptr)
                streamName = streamReq->getStreamName();
            break;
        }
        case 'i': {
            auto streamInd = packet->findTag<StreamInd>();
            if (streamInd != nullptr)
                streamName = streamInd->getStreamName();
            break;
        }
        case 'b': {
            auto streamReq = packet->findTag<StreamReq>();
            if (streamReq != nullptr)
                streamName = streamReq->getStreamName();
            else {
                auto streamInd = packet->findTag<StreamInd>();
                if (streamInd != nullptr)
                    streamName = streamInd->getStreamName();
            }
            break;
        }
    }
    if (streamName != nullptr && mapping->containsKey(streamName))
        return mapping->get(streamName).intValue() + gateIndexOffset;
    else
        return defaultGateIndex;
}

} // namespace inet

