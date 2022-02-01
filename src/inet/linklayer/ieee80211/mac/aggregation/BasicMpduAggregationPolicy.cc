//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/BasicMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicMpduAggregationPolicy);

void BasicMpduAggregationPolicy::initialize()
{
}

std::vector<Packet *> *BasicMpduAggregationPolicy::computeAggregateFrames(std::vector<Packet *> *frames)
{
    Enter_Method("computeAggregateFrames");
    return nullptr;
}

} /* namespace ieee80211 */
} /* namespace inet */

