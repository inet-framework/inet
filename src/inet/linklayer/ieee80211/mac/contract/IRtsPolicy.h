//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRTSPOLICY_H
#define __INET_IRTSPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRtsPolicy
{
  public:
    virtual ~IRtsPolicy() {}

    virtual bool isRtsNeeded(Packet *packet, const Ptr<const Ieee80211MacHeader>& protectedHeader) const = 0;
    virtual simtime_t getCtsTimeout(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const = 0;
    virtual int getRtsThreshold() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

