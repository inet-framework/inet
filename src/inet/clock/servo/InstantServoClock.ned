//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

// As simple clock servo (previously known as SettableClock in INET), with three different modes:
//
// 1. No servo: Calls to adjustTime are ignored. (adjustClock = false; adjustDrift = false)
//
// 2. Jump only: Calls to adjustTime are used to jump the clock. (adjustClock = true; adjustDrift = false)
//
// 3. Jump and estimate drift: Calls to adjustTime are used to jump the clock and estimate the drift using the previous
// call to adjustTime. (adjustClock = true; adjustDrift = true)
module InstantServoClock extends ServoClockBase
{
    parameters:
        bool adjustClock = default(true); // Whether to adjust the clock.
        bool adjustDrift = default(true); // Whether to adjust the drift.
        @class(InstantServoClock);
}
