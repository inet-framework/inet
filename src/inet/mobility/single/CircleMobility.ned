//
// Copyright (C) 2005 Andras Varga
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

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Moves the node around a circle.
//
// @author Andras Varga
//
simple CircleMobility extends MovingMobilityBase
{
    parameters:
        double cx @unit(m); // x coord of the center of the circle
        double cy @unit(m); // y coord of the center of the circle
        double cz @unit(m) = default(0m); // z coord of the center of the circle
        double r @unit(m); // radius of the circle
        double speed @unit(mps) = default(0mps); // speed of the host
        double startAngle @unit(deg) = default(uniform(0deg, 360deg)); // starting angle
        @class(CircleMobility);
}
