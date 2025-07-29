//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

import inet.common.Module;

//
// Base module for clocks.
//
// Display string format directives:
//
//  -t: clock time in seconds
//  -T: clock time in automatically selected (natural) units
//  -d: clock time difference to reference clock in automatically selected (natural) units
//
module ClockBase extends Module
{
    parameters:
        displayStringTextFormat = default("%t"); // Determines the text that is written on top of the submodule
        string referenceClock = default("");            // Full path of the reference clock
        int clockTimeChangeEventSchedulingPriority = default(0); // Specifies the scheduling priority of the clock time changed event
        double emitClockTimeInterval @unit(s) = default(0s); // A non-zero value means the clock time will be emitted periodically
        @class(ClockBase);
        @display("i=block/timer");
        @signal[timeChanged](type=simtime_t);
        @statistic[timeChanged](title="Clock time"; source=localSignal(timeChanged); record=vector; interpolationmode=linear; unit=s);
}

