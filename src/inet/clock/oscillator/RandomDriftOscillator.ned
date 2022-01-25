//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// This oscillator changes drift rate over time. The drift rate is the sum of
// a distribution that is evaluated periodically and a random walk process.
//
simple RandomDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        volatile double changeInterval @unit(s); // drift change happens at the end of every interval
        volatile double driftRate @unit(ppm) = default(0ppm); // expressed as a ratio in parts per million
        volatile double driftRateChange @unit(ppm) = default(0ppm); // integrated over time (random walk), no change by default
        double driftRateChangeLowerLimit @unit(ppm) = default(-inf ppm); // lower limit for random walk, no limit by default
        double driftRateChangeUpperLimit @unit(ppm) = default(inf ppm); // upper limit for random walk, no limit by default
        @class(RandomDriftOscillator);
}

