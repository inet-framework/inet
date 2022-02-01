//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IACKHANDLER_H
#define __INET_IACKHANDLER_H

namespace inet {
namespace ieee80211 {

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

class INET_API IAckHandler
{
  public:
    virtual ~IAckHandler() {}

    virtual bool isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
    virtual bool isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
    virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

