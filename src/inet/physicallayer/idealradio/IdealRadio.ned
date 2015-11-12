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

package inet.physicallayer.idealradio;

import inet.physicallayer.common.packetlevel.Radio;

//
// This radio model provides a very simple but fast and predictable physical
// layer behavior. It must be used in conjunction with the ~IdealRadioMedium
// model.
//
// The idea behind this radio model is to describe transmissions with a few
// distance based parameters: communication range, interference range, and
// detection range. Whether the reception is successful or not, depends on the
// distance between the transmitter and the receiver. This model is similar to
// the unit disk radio, but it's also more general than that. The ideal model
// also supports an ideal communication channel where all transmissions are
// received sucessfully independent of distance and interfering transmissions.
//
// See also ~IdealRadioMedium, ~IdealAnalogModel, ~IdealTransmitter, and
// ~IdealReceiver.
//
module IdealRadio extends Radio
{
    parameters:
        antennaType = default("IsotropicAntenna");
        transmitterType = default("IdealTransmitter");
        receiverType = default("IdealReceiver");
        separateTransmissionParts = default(false);
        separateReceptionParts = default(false);
}
