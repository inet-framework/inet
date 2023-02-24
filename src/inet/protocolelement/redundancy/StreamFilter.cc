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
    if (stage == INITSTAGE_LOCAL)
        streamNameFilter.setPattern(par("streamNameFilter"), false, true, true);
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
    auto streamReq = packet->findTag<StreamReq>();
    auto streamInd = packet->findTag<StreamInd>();
    if (streamReq != nullptr) {
        auto streamName = streamReq->getStreamName();
        cMatchableString matchableString(streamName);
        return const_cast<cMatchExpression *>(&streamNameFilter)->matches(&matchableString);
    } else if (streamInd != nullptr) {
        auto streamName = streamInd->getStreamName();
        cMatchableString matchableString(streamName);
        return const_cast<cMatchExpression *>(&streamNameFilter)->matches(&matchableString);
    }
    return false;
}

} // namespace inet

