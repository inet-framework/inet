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

//
// Models a hypotetical antenna with a cosine-based radiation pattern. 
// This antenna model is commonly used in the real world to approximate
// various directional antennas.
//
// Gain is computed from the angle as follows:
// <pre>
//   exponent = -3 / (20 * log10(cos(beamWidth) / 4))
//   gain = maxGain * abs(cos(angle / 2)) ^ exponent
// </pre>
// This module represents the cosine antenna model found in 
//
// Li Chunjian, "Efficient Antenna Patterns for Three-Sector WCDMA Systems",
// eq. B.1 and B.2
//
module CosineAntenna extends AntennaBase
{
    parameters:
        double maxGain @unit(dB);    // gain at the antenna boresight
        double beamWidth @unit(deg); // 3dB beam width
        @class(CosineAntenna);
}

