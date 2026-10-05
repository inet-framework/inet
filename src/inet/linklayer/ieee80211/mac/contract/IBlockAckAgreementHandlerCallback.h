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

    // Recompute the shared timer from the absolute deadlines in both agreement handlers.
    virtual void scheduleInactivityTimer() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

