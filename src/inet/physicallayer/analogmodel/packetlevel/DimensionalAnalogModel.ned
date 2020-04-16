//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.analogmodel.packetlevel;

import inet.physicallayer.contract.packetlevel.IAnalogModel;

//
// This analog model computes with dimensional analog power representation. In
// other words the signal power may change over time and/or frequency.
//
module DimensionalAnalogModel like IAnalogModel
{
    parameters:
        bool attenuateWithCenterFrequency = default(true); // true means frequency selectivity will always use the carrier frequencey as opposed to using the frequency where the change in power occurs
        @display("i=block/tunnel");
        @class(DimensionalAnalogModel);
}

