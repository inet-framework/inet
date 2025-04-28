//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybová, Vladimir Veselý (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.tables;

import inet.common.SimpleModule;

//
// The `PimNeighborTable` module contains information about neighboring PIM routers.
//
simple PimNeighborTable extends SimpleModule
{
    parameters:
        @class(PimNeighborTable);
        @display("i=block/table");
        @signal[pimNeighborAdded](type=inet::PimNeighbor);
        @signal[pimNeighborDeleted](type=inet::PimNeighbor);
        @signal[pimNeighborChanged](type=inet::PimNeighbor);
}

