//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.base.ClockBase;
import inet.clock.contract.IClock;
import inet.clock.contract.IOscillator;

//
// Models a clock that uses an oscillator to produce ticks periodically. The
// oscillator is allowed to drift and produce ticks with different lengths over
// time. The clock simply counts the number of oscillator ticks. The mapping
// between clock ticks and clock time uses the nominal tick length.
//
// When scheduling to a clock time or with a clock delay, this clock rounds the
// argument value using the specified rounding method first.
//
// Display string format directives:
//
//  -t: time in seconds
//  -T: time in natural units
//  -d: time difference to reference clock in natural units
//  -s: origin simulation time
//  -c: origin clock time
//

module OscillatorBasedClock extends ClockBase like IClock
{
    parameters:
        displayStringTextFormat = default("%t (%c)"); // determines the text that is written on top of the submodule
        double initialClockTime @unit(s) = default(0s); // clock time at the simulation time when the module is initialized
        string roundingMode @enum("down","up","closer","none") = default("up"); // when scheduling to a clock time or with a clock delay this clock uses rounding with respect to the nominal tick length
        @class(OscillatorBasedClock);
    submodules:
        oscillator: <default("ConstantDriftOscillator")> like IOscillator {
            @display("p=100,100");
        }
}

