//
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//

package inet.mobility.static;

import inet.mobility.common.MobilityBase;


//
// Places all hosts in a rectangular grid.
// The usable area (constraint area minus margins on each side) is split into smaller cells
// (with separationX,separationY size). Hosts are placed in the middle of each cell.
// By default, the number of columns and rows follow the aspect ratio of the usable area.
// By default stepX and stepY are calculated based on the number of columns and rows.
//
simple StaticGridMobility extends MobilityBase
{
    parameters:
        int marginX @unit(m) = default(0m);
        int marginY @unit(m) = default(0m);
        int numHosts;
        int columns = default(ceil(sqrt(numHosts * (this.constraintAreaMaxX - this.constraintAreaMinX - 2 * marginX) / (this.constraintAreaMaxY - this.constraintAreaMinY - 2 * marginY))));
        int rows = default((numHosts + columns - 1) / columns);
        double initialZ @unit(m) = default(0m);
        double separationX @unit(m) = default((this.constraintAreaMaxX - this.constraintAreaMinX - 2 * marginX) / columns);
        double separationY @unit(m) = default((this.constraintAreaMaxY - this.constraintAreaMinY - 2 * marginY) / rows);
        @class(StaticGridMobility);
}

