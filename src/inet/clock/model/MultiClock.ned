//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.contract.IClock;

//
// Contains several subclocks and also implements the clock module
// interface itself. It exposes the currently active clock to its users. The
// active clock can be changed programmatically. This module is primarily useful
// for multi-domain gPTP time synchronization.
//
// @see ~MultiDomainGptp
//
module MultiClock like IClock
{
    parameters:
        int numClocks; // Number of subclocks
        int activeClockIndex @mutable = default(0); // The currently active subclock
        @display("i=block/timer");
        @class(MultiClock);
        @signal[timeChanged](type=simtime_t);
        @signal[timeJumped](); // Signal is emitted when the activeClock performs a timeJump or the activeClockIndex updates
        @signal[timeDifferenceToReference](type=simtime_t);
        @signal[activeClockIndexChanged](type=int);
        @statistic[timeChanged](title="Clock time"; source=localSignal(timeChanged); record=vector; interpolationmode=linear);
        @statistic[timeDifferenceToReference](title="Time difference to reference"; source=localSignal(timeDifferenceToReference); record=vector; interpolationmode=linear);
        @statistic[activeClockIndexChanged](title="Active clock index"; source=activeClockIndexChanged; record=vector; interpolationmode=sample-hold);
    submodules:
        clock[numClocks]: <default("PiServoClock")> like IClock {
            @display("p=200,200,row,200");
        }
}

