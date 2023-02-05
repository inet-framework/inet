//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

//
// This is a base module for clocks.
//
module ClockBase
{
    parameters:
        string displayStringTextFormat = default("%t"); // determines the text that is written on top of the submodule
        string referenceClock = default("");			// full path of reference clock
        double emitClockTimeInterval @unit(s) = default(0s); // non-zero value means the clock time will be emitted periodically
        @class(ClockBase);
        @display("i=block/timer");
        @signal[timeChanged](type=simtime_t);
        @statistic[timeChanged](title="Clock time"; record=vector; interpolationmode=linear);
}

