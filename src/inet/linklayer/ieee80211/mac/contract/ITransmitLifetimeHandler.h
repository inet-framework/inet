//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRANSMITLIFETIMEHANDLER_H
#define __INET_ITRANSMITLIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API ITransmitLifetimeHandler
{
  public:
    virtual ~ITransmitLifetimeHandler() {}

    virtual void frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header) = 0;
    virtual void frameTransmitted(const Ptr<const Ieee80211DataHeader>& header) = 0;

    virtual bool isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

