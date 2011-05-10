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


package inet.mobility.models;

import inet.mobility.MobilityBase;


//
// This mobility module does nothing; it can be used for stationary nodes.
//
// @author Andras Varga
//
simple NullMobility extends MobilityBase
{
    parameters:
        double initialX @unit("m") = default(uniform(constraintAreaX, constraintAreaX + constraintAreaWidth)); // start x coordinate
        double initialY @unit("m") = default(uniform(constraintAreaY, constraintAreaY + constraintAreaHeight)); // start y coordinate
        bool initFromDisplayString = default(true);
        @display("i=block/cogwheel_s");
        @class(NullMobility);
}



