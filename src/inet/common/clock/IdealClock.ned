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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.common.clock;

import inet.common.clock.contract.IClock;

//
// Models a clock where the drift rate is constant.
//
simple LinearClock like IClock
{
    parameters:
        double origin @unit(s) = default(0s); // the simulation time when the clock's value is zero
        //TODO rename to: offset? startTime? timeShift? 
        double driftRate @unit(ppm) = default(0ppm); // clock precision, expressed as a ratio in parts per million
        @display("i=block/timer");
}

