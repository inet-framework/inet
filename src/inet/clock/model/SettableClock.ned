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

//
// Models a clock which can be set to a different clock time. The clock time
// can be set from C++ or using a <set-clock module="..." time="..."/> command
// in a ScenarioManager script.
//
// @see ~ScenarioManager
//
module SettableClock extends OscillatorBasedClock
{
    parameters:
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("error");
        @class(SettableClock);
}

