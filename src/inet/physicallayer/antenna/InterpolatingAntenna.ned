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
// This antenna model computes the antenna gain from the direction of the signal
// using linear interpolation for all 3 euler angles independently of each other.
// The gain parameters contain a sequence of angle [degree] and gain [dB] pairs.
// The first pair must be at 0 [degree].
//
//# TODO: refactor to use Delaunay triangulation on unit sphere, i.e. triangulate
//# result from enclosing spherical triangle as seen from the center
//
module InterpolatingAntenna extends AntennaBase
{
    parameters:
        string headingGains = default("0 0");   // sequence of angle [degree] and gain [dB] pairs
        string elevationGains = default("0 0"); // sequence of angle [degree] and gain [dB] pairs
        string bankGains = default("0 0");      // sequence of angle [degree] and gain [dB] pairs
        @class(InterpolatingAntenna);
}

