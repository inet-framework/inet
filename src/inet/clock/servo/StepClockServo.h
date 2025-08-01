//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STEPCLOCKSERVO_H
#define __INET_STEPCLOCKSERVO_H

#include "inet/clock/base/ClockServoBase.h"

namespace inet {

class INET_API StepClockServo : public ClockServoBase
{
  public:
    virtual void adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference) override;
};

} // namespace inet

#endif

