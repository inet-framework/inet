//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SETTABLECLOCK_H
#define __INET_SETTABLECLOCK_H

#include "inet/clock/model/ServoClockBase.h"



namespace inet {

class INET_API InstantServoClock : public ServoClockBase
{
  protected:
    long offsetPrev = 0;
    long localPrev = 0;
    ppm drift = ppm(0);
  public:
    virtual void adjustClockTo(clocktime_t newClockTime) override;
};

} // namespace inet

#endif
