//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMSDUDEAGGREGATION_H
#define __INET_IMSDUDEAGGREGATION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IMsduDeaggregation
{
  public:
    virtual ~IMsduDeaggregation() {}

    // Inspect lengths without consuming the packet; callers may use this before success signals.
    virtual bool isValidAggregate(const Packet *frame) const = 0;

    // Success consumes frame and transfers ownership of the returned vector and packets.
    // Malformed lengths return nullptr; frame remains unchanged and caller-owned.
    virtual std::vector<Packet *> *deaggregateFrame(Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

