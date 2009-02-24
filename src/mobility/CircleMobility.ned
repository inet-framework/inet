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

package inet.mobility;

//
// Moves the node around a circle.
//
// @see ChannelControl
// @author Andras Varga
//
simple CircleMobility like BasicMobility
{
    parameters:
        bool debug = default(false); // debug switch
        double cx = default(100); // x coord of the center of the circle
        double cy = default(100); // y coord of the center of the circle
        double r = default(100); // radius of the circle
        double speed @unit("mps") = default(2mps); // speed of the host (in m/s)
        double startAngle @unit("deg") = default(0); // starting angle (degreees)
        double updateInterval @unit("s") = default(100ms); // time interval to update the hosts position
        @display("i=block/cogwheel_s");
}

