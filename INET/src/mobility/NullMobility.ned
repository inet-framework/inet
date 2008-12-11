//
// Copyright (C) 2006 Andras Varga
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
// This mobility module does nothing; it can be used for stationary nodes.
//
// @author Andras Varga
//
simple NullMobility like BasicMobility
{
    parameters:
        double x = default(-1); // start x coordinate (-1 = display string position, or random if it's missing)
        double y = default(-1); // start y coordinate (-1 = display string position, or random if it's missing)
        @display("i=block/cogwheel_s");
}



