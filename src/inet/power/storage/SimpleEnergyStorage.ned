//
// Copyright (C) 2013 OpenSim Ltd.
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

import inet.power.base.EnergyStorageBase;

//
// This energy storage model maintains a residual capacity by integrating the
// difference between the total absorbed power and the total provided power
// over time. It can initiate node shutdown when the residual capacity drops
// below a configured threshold. It can also initiate node start when the
// residual capacity raises above another configured threshold.
//
// This model doesn't have various properties such as self-discharge, memory
// effect, overcharging, temperature-dependence, etc. that real world batteries
// have.
//
// @author Levente Meszaros
//
simple SimpleEnergyStorage extends EnergyStorageBase
{
    double nominalCapacity @unit(J);
    double initialCapacity @unit(J) = default(nominalCapacity);
    double printCapacityStep @unit(J) = default(0.01 * nominalCapacity);
    double nodeShutdownCapacity @unit(J) = default(0J/0);
    double nodeStartCapacity @unit(J) = default(0J/0);
    @class(SimpleEnergyStorage);
}
