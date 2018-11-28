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

package inet.power.management;

import inet.power.contract.IEpEnergyManagement;

//
// This energy mangement model estimates the residual energy capacity of the
// energy source model by actually querying it. It is only useful when the
// estimation process is not important. This model initiates node shutdown
// when the residual capacity decreases below a threshold, and it also
// initiates node start when the residual capacity increases above another
// threshold.
//
// @see ~SimpleEpEnergyStorage
// @author Levente Meszaros
//
simple SimpleEpEnergyManagement like IEpEnergyManagement
{
    parameters:
        double nodeShutdownCapacity @unit(J) = default(nan J); // initiates node shutdown when the residual energy capacity decreases below this value
        double nodeStartCapacity @unit(J) = default(nan J); // initiates node start when the residual energy capacity increases above this value
        @class(SimpleEpEnergyManagement);
        @display("i=block/plug");
}

