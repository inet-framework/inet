//
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

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
        double marginX @unit(m) = default(0m);
        double marginY @unit(m) = default(0m);
        int numHosts;
        int columns = default(int(ceil(sqrt(numHosts * (this.constraintAreaMaxX - this.constraintAreaMinX - 2 * marginX) / (this.constraintAreaMaxY - this.constraintAreaMinY - 2 * marginY)))));
        int rows = default(int((numHosts + columns - 1) / columns));
        double initialZ @unit(m) = default(0m);
        double separationX @unit(m) = default((this.constraintAreaMaxX - this.constraintAreaMinX - 2 * marginX) / columns);
        double separationY @unit(m) = default((this.constraintAreaMaxY - this.constraintAreaMinY - 2 * marginY) / rows);
        @class(StaticGridMobility);
}

