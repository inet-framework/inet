//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMPDUDEAGGREGATION_H
#define __INET_IMPDUDEAGGREGATION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IMpduDeaggregation
{
  public:
    virtual ~IMpduDeaggregation() {}

    virtual std::vector<Packet *> *deaggregateFrame(Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

