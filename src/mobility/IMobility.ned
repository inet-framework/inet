//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.mobility;

//
// This is not an actual mobility model, but a prototype for other mobility models.
//
// @author Andras Varga
//
moduleinterface IMobility
{
    parameters:
        @display("i=block/cogwheel");
}

simple MobilityBase like IMobility
{
    parameters:
        double constraintAreaX @unit("m") = default(0m); // topleft x position of the playground (in meters)
        double constraintAreaY @unit("m") = default(0m); // topleft y position of the playground (in meters)
        double constraintAreaWidth @unit("m") = default(600m); // x size of the playground (in meters)
        double constraintAreaHeight @unit("m") = default(400m); // y size of the playground (in meters)
        @display("i=block/cogwheel");
}

