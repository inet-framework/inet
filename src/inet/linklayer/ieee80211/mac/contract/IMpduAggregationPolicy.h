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
namespace physicallayer { class IIeee80211Mode; }
namespace ieee80211 {

struct INET_API MpduAggregationLimits
{
    MacAddress receiver;
    MacAddress transmitter;
    Tid tid = -1;
    SequenceNumberCyclic windowStart;
    int windowSize = 0;
    B maxLength = B(0);
    int minimumMpduStartSpacing = 0; // Encoded peer receiver limit, 0..7.
    const physicallayer::IIeee80211Mode *mode = nullptr;
};

class INET_API IMpduAggregationPolicy
{
  public:
    virtual ~IMpduAggregationPolicy() {}

    virtual std::vector<Packet *> *computeAggregateFrames(std::vector<Packet *> *frames) = 0;
    virtual bool isEnabled() const = 0;
    // The result borrows an eligible prefix of the supplied candidates. No source state changes.
    virtual std::vector<Packet *> selectFrames(const std::vector<Packet *>& candidates, const MpduAggregationLimits& limits) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

