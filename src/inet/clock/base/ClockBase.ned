//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

//
// Base module for clocks.
//
module ClockBase
{
    parameters:
        string displayStringTextFormat = default("%t"); // Determines the text that is written on top of the submodule
        string referenceClock = default("");            // Full path of the reference clock
        double emitClockTimeInterval @unit(s) = default(0s); // A non-zero value means the clock time will be emitted periodically
        @class(ClockBase);
        @display("i=block/timer");
        @signal[timeChanged](type=simtime_t);
        @statistic[timeChanged](title="Clock time"; record=vector; interpolationmode=linear);
}

