//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.neighborcache;

import inet.physicallayer.wireless.common.contract.packetlevel.INeighborCache;

//
// This neighbor cache model maintains a separate periodically updated neighbor
// list for each radio.
//
module NeighborListNeighborCache like INeighborCache
{
    parameters:
        string radioMediumModule = default("^");
        double range @unit(m);
        double refillPeriod @unit(s);
        @display("i=block/table2");
        @class(NeighborListNeighborCache);
}

