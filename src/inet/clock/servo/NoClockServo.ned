//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.base.ClockServoBase;
import inet.clock.contract.IClockServo;

//
// This module implements a clock servo which does not update the clock.
//
module NoClockServo extends ClockServoBase like IClockServo
{
    parameters:
        @class(NoClockServo);
}

