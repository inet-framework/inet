//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.power.storage;

import inet.power.base.EpEnergyStorageBase;

//
// This energy storage model maintains a residual energy capacity by integrating
// the difference between the total consumed power and the total generated power
// over time. It initiates node crash when the residual energy capacity reaches
// zero. This model doesn't have various properties such as self-discharge, memory
// effect, overcharging, temperature-dependence, etc. that real world batteries
// have. See the base module for signals and statistics.
//
// @see ~SimpleEpEnergyManagement
// @author Levente Meszaros
//
simple SimpleEpEnergyStorage extends EpEnergyStorageBase
{
    parameters:
        double nominalCapacity @unit(J); // the maximum amount of energy stored
        double initialCapacity @unit(J) = default(this.nominalCapacity); // the initially stored amount of energy
        double printCapacityStep @unit(J) = default(0.01 * this.nominalCapacity); // displays residual capacity changes in the log
        string displayStringTextFormat = default("%c (%p)");
        @class(SimpleEpEnergyStorage);
}

