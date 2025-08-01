//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.base.ClockServoBase;
import inet.clock.contract.IClockServo;

//
// This module implements a clock servo that immediately sets the clock time
// and applies the new oscillator compensation. This can result in abrupt time
// jumps, including backward adjustments, which may disrupt other modules that
// depend on monotonic clock progression.
//
module StepClockServo extends ClockServoBase like IClockServo
{
    parameters:
        @class(StepClockServo);
}

