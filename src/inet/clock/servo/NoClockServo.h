//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NOCLOCKSERVO_H
#define __INET_NOCLOCKSERVO_H

#include "inet/clock/base/ClockServoBase.h"

namespace inet {

class INET_API NoClockServo : public ClockServoBase
{
  public:
    virtual void adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference) override;
};

} // namespace inet

#endif

