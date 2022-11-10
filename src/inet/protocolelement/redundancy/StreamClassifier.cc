//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    if (streamName != nullptr && mapping->containsKey(streamName)) {
        int outputGateIndex = mapping->get(streamName).intValue() + gateIndexOffset;
        if (consumers[outputGateIndex]->canPushPacket(packet, outputGates[outputGateIndex]->getPathEndGate()))
            return outputGateIndex;
    }
    return defaultGateIndex;
}

} // namespace inet

