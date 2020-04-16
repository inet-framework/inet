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

package inet.physicallayer.apskradio.packetlevel;

import inet.physicallayer.base.packetlevel.FlatRadioBase;

//
// This radio model provides a hypothetical radio that simply uses one of the
// well-known modulations without utilizing other techiques such as forward
// error correction, interleaving, spreading, etc.
//
// @see ~ApskScalarRadio, ~ApskScalarRadioMedium.
//
module ApskRadio extends FlatRadioBase
{
    parameters:
        string signalAnalogRepresentation @enum("scalar", "dimensional") = default("scalar");
        transmitter.typename = default(signalAnalogRepresentation == "scalar" ? "ApskScalarTransmitter" : signalAnalogRepresentation == "dimensional" ? "ApskDimensionalTransmitter" : "");
        receiver.typename = default(signalAnalogRepresentation == "scalar" ? "ApskScalarReceiver" : signalAnalogRepresentation == "dimensional" ? "ApskDimensionalReceiver" : "");
        @class(ApskRadio);
}

