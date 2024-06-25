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
    clocktime_t offset[2];
    clocktime_t local[2];
    int phase = 0;
    ppm drift = ppm(0);
  public:
    /**
     * Sets the clock time immediately to the given value. Greater than 1 oscillator
     * compensation factor means the clock measures time faster.
     */
    virtual void adjustClockTo(clocktime_t newClockTime) override;

};

} // namespace inet

#endif
