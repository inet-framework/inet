//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Ampdu.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduAggregation);

Packet *MpduAggregation::aggregateFrames(std::vector<Packet *> *frames)
{
    return frames->empty() ? new Packet() : aggregateFrames(frames,{});
}

Packet *MpduAggregation::aggregateFrames(std::vector<Packet *> *frames, const MpduAggregationParameters& parameters)
{
    auto result = buildAmpdu(*frames,parameters.vht,parameters.minimumStartDistance);
    for (auto frame : *frames)
        delete frame;
    return result;
}

} /* namespace ieee80211 */
} /* namespace inet */

