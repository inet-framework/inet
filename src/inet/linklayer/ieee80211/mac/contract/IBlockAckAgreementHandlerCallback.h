//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H
#define __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"

namespace inet {
namespace ieee80211 {

class INET_API IBlockAckAgreementHandlerCallback
{
  public:
    virtual ~IBlockAckAgreementHandlerCallback() {}

    virtual void blockAckAgreementTerminated(cObject *agreement) = 0;
    virtual void rescheduleInactivityTimer() = 0;
    virtual void recipientAgreementTerminated(MacAddress originatorAddress, Tid tid) = 0;

    // Deprecated for one release. The deadline owners supply the schedule.
    virtual void scheduleInactivityTimer(simtime_t timeout) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

