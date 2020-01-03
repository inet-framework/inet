//
// Copyright (C) OpenSim Ltd
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

package inet.physicallayer.apskradio.bitlevel;

import inet.physicallayer.apskradio.packetlevel.ApskRadio;

//
// This radio model is part of a simple hypothetical layered radio. It
// produces detailed transmissions that have separate representation for
// all simulated domains. It must be used in conjunction with the
// ~ApskLayeredScalarRadioMedium model.
//
// @see ~ApskLayeredScalarRadioMedium, ~LayeredScalarAnalogModel,
// ~ApskLayeredTransmitter, ~ApskLayeredReceiver.
//
module ApskLayeredScalarRadio extends ApskRadio
{
    parameters:
        transmitter.typename = default("ApskLayeredTransmitter");
        receiver.typename = default("ApskLayeredReceiver");
}
