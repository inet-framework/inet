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

package inet.clock.oscillator;

import inet.clock.base.DriftingOscillatorBase;
import inet.clock.contract.IOscillator;

//
// This module generates ticks periodically using a constant drift in the clock
// speed. The first tick is at the simulation time of the module initialization
// optionally shifted with an offset. The constant drift can be set from C++ or
// using a <set-oscillator module="..." drift-rate="..." tick-offset=".."/>
// command in a ScenarioManager script.
//
// @see ~ScenarioManager
//
simple ConstantDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double driftRate @unit(ppm) = default(0ppm); // expressed as a ratio in parts per million
        @class(ConstantDriftOscillator);
}

