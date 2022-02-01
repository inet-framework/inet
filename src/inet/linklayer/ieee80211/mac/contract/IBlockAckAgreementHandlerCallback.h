//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H
#define __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class INET_API IBlockAckAgreementHandlerCallback
{
  public:
    virtual ~IBlockAckAgreementHandlerCallback() {}

    virtual void scheduleInactivityTimer(simtime_t timeout) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

