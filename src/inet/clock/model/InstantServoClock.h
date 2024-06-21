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

  public:
    /**
     * Sets the clock time immediately to the given value. Greater than 1 oscillator
     * compensation factor means the clock measures time faster.
     */
    virtual void adjustClockTo(clocktime_t newClockTime) override;

};

} // namespace inet

#endif
