//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.power.storage;

import inet.power.base.CcEnergyStorageBase;

//
// This battery model maintains a residual charge capacity by integrating the
// difference between the total consumed current and the total generated current
// over time. This model uses a charge independent ideal voltage source and an
// charge independent internal resistance. It initiates node crash when the
// residual charge capacity reaches zero. See the base module for signals and
// statistics.
//
// @author Levente Meszaros
//
simple SimpleCcBattery extends CcEnergyStorageBase
{
    parameters:
        double nominalCapacity @unit(C); // maximum amount of charge stored
        double initialCapacity @unit(C); // the initially stored amount of charge
        double nominalVoltage @unit(V); // open circuit voltage at maximum charge
        double internalResistance @unit(Ohm); // internal resistance
        string displayStringTextFormat = default("%c (%p)");
        @class(SimpleCcBattery);
}

