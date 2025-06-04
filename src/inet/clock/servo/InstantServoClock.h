//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_INSTANTSERVOCLOCK_H
#define __INET_INSTANTSERVOCLOCK_H

#include "ServoClockBase.h"

namespace inet {

class INET_API InstantServoClock : public ServoClockBase
{
  protected:
    long offsetPrev = 0;
    long localPrev = 0;
    ppm drift = ppm(0);
    bool adjustClock = true;
    bool adjustDrift = true;

  public:
    virtual void adjustClockTo(clocktime_t newClockTime) override;
    virtual void initialize(int stage) override;
    virtual void resetClockState() override;
};

} // namespace inet

#endif
