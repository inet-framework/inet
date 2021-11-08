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

import inet.clock.contract.IClock;

//
// This module contains several subclocks and also implements the clock module
// interface itself. It exposes the currently active clock to its users. The
// active clock can be changed programmatically. This module is primarily useful
// for multi domain gPTP time synchronization.
//
// @see ~MultiDomainGptp
//
module MultiClock like IClock
{
    parameters:
        int numClocks; // number of subclocks
        int activeClockIndex @mutable = default(0); // the currently active subclock
        @display("i=block/timer");
        @class(MultiClock);
        @signal[timeChanged](type=simtime_t);
        @statistic[timeChanged](title="Clock time"; source=localSignal(timeChanged); record=vector; interpolationmode=linear);
    submodules:
        clock[numClocks]: <default("SettableClock")> like IClock {
            @display("p=200,200,row,200");
        }
}

