//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMSDUAGGREGATIONPOLICY_H
#define __INET_IMSDUAGGREGATIONPOLICY_H

#include <functional>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace ieee80211 {

class INET_API IMsduAggregationPolicy
{
  public:
    virtual ~IMsduAggregationPolicy() {}

    // A non-null result is caller-owned and must contain at least two unique,
    // discoverable, eligible frames with candidate as its first element.
    virtual std::vector<Packet *> *computeAggregateFrames(queueing::IPacketQueue *queue, Packet *candidate, const std::function<bool(const Packet *)>& isFrameEligible) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
