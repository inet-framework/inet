//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICLOCKSERVO_H
#define __INET_ICLOCKSERVO_H

#include "inet/clock/contract/IClock.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * This class defines the interface for clock servos. See the corresponding NED file for details.
 */
class INET_API IClockServo
{
  public:
    virtual ~IClockServo() {}

    /**
     * Requests the clock servo to adjust the clock based on the clock time difference and oscillator
     * rate difference between the clock controlled by this clock servo and the reference clock.
     * Positive timeDifference means this clock is ahead of the reference clock, and positive
     * rateDifference means this clock is faster than the reference clock.
     */
    virtual void adjustClockForDifference(clocktime_t timeDifference, ppm rateDifference) = 0;
};

} // namespace inet

#endif

