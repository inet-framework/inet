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

bool StreamFilter::matchesPacket(const Packet *packet) const
{
    auto streamReq = packet->findTag<StreamReq>();
    if (streamReq != nullptr) {
        auto streamName = streamReq->getStreamName();
        cMatchableString matchableString(streamName);
        return const_cast<cMatchExpression *>(&streamNameFilter)->matches(&matchableString);
    }
    return false;
}

} // namespace inet

