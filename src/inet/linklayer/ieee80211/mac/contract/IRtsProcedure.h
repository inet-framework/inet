//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRTSPROCEDURE_H
#define __INET_IRTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRtsProcedure
{
  public:
    virtual ~IRtsProcedure() {}

    virtual const Ptr<Ieee80211RtsFrame> buildRtsFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const = 0;
    virtual void processTransmittedRts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

