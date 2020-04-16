//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.unitdisk;

import inet.physicallayer.contract.packetlevel.IAnalogModel;

//
// This analog model provides a very simple and predictable physical layer
// behavior. It determines the reception power by categorizing transmissions
// based on the distance between the transmitter and the receiver. It doesn't
// support noise and signal to noise ratio calculations.
//
// @see ~UnitDiskRadio, ~UnitDiskRadioMedium, ~UnitDiskTransmitter,
// ~UnitDiskReceiver.
//
module UnitDiskAnalogModel like IAnalogModel
{
    parameters:
        @class(UnitDiskAnalogModel);
        @display("i=block/tunnel");
}

