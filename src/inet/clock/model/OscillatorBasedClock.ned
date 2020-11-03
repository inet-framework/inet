//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

