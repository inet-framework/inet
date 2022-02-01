//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMSDUAGGREGATIONPOLICY_H
#define __INET_IMSDUAGGREGATIONPOLICY_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace ieee80211 {

class INET_API IMsduAggregationPolicy
{
  public:
    virtual ~IMsduAggregationPolicy() {}

    virtual std::vector<Packet *> *computeAggregateFrames(queueing::IPacketQueue *queue) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

