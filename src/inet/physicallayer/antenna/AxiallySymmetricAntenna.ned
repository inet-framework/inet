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

package inet.physicallayer.antenna;

import inet.physicallayer.base.packetlevel.AntennaBase;

// This antenna model computes the antenna gain from the direction using linear
// interpolation. The gain parameter contains a sequence of angle [degree] and
// gain [dB] pairs. The first pair must be at 0 [degree].
//
module AxiallySymmetricAntenna extends AntennaBase
{
    parameters:
        double baseGain @unit(dB) = default(0dB);
        string gains = default("0 0"); // sequence of angle [degree] in the range (0, 180) and gain [dB] pairs
        string axisOfSymmetry = default("x"); // allows matching the antenna radiation pattern (IAntennaGain) to the visual representation, antenna orientation is determined by the antenna's mobility model
        @class(AxiallySymmetricAntenna);
}

