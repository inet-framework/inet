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
// This antenna model describes the well-known dipole antenna or doublet. It
// consists of two identical conductive elements, which are bilaterally
// symmetrical.
//
module DipoleAntenna extends AntennaBase
{
    parameters:
        double length @unit(m); // the length of the dipole antenna
        string wireAxis = default("z"); // allows matching the antenna radiation pattern (IAntennaGain) to the visual representation, antenna orientation is determined by the antenna's mobility model
                                        // the default value is Z axis, so that antenna mobility models are not required when antennas are approximately positioned in the same plane
        @class(DipoleAntenna);
}

