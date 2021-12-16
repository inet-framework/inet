//
// Copyright (C) 2004 OpenSim Ltd.
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

package inet.applications.clock;

import inet.applications.contract.IApp;

//
// This module periodically synchronizes the slave clock to the master clock with
// the specified accuracy. The synchronization happens directly using C++ methods
// calls and without any packet exchange. This is primarily useful when the overhead
// of the time synchronization protocol messages can be safely ignored.
//
simple SimpleClockSynchronizer like IApp
{
    parameters:
        string masterClockModule;
        string slaveClockModule;
        volatile double synchronizationInterval @unit(s); // clock synchronization period
        volatile double synchronizationAccuracy @unit(s) = default(0s); // 0 means exact synchronization
        @display("i=block/app");
        @lifecycleSupport;
    gates:
        input socketIn @loose;
        output socketOut @loose;
}

