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

package inet.clock.contract;

//
// This module interface is implemented by oscillator models. Oscillators are
// typically simple modules, and are used by other modules via direct C++ method
// calls.
//
// The IOscillator C++ interface provides an API to efficiently simulate the
// generation of ticks that are usually counted by clocks. The tick period is
// not necessarily constant, it can change over time.
//
// @see ~IClock
//
moduleinterface IOscillator
{
    parameters:
        @display("i=block/tunnel");
}

