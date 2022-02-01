//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMPDUAGGREGATIONPOLICY_H
#define __INET_IMPDUAGGREGATIONPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IMpduAggregationPolicy
{
  public:
    virtual ~IMpduAggregationPolicy() {}

    virtual std::vector<Packet *> *computeAggregateFrames(std::vector<Packet *> *frames) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

