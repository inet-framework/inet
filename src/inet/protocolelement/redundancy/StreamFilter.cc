//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamFilter.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamFilter);

void StreamFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mode = par("mode");
        streamNameFilter.setPattern(par("streamNameFilter"), false, true, true);
    }
}

cGate *StreamFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

bool StreamFilter::matchesPacket(const Packet *packet) const
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
    if (streamName != nullptr) {
        cMatchableString matchableString(streamName);
        return const_cast<cMatchExpression *>(&streamNameFilter)->matches(&matchableString);
    }
    else
        return false;
}

} // namespace inet

