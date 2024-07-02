//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.neighborcache;

import inet.physicallayer.wireless.common.contract.packetlevel.INeighborCache;

//
// This neighbor cache model organizes radios in a two-dimensional quad tree
// (ignoring the Z axis) with a constant node size and updates periodically.
//
module QuadTreeNeighborCache like INeighborCache
{
    parameters:
        string radioMediumModule = default("^");
        int maxNumOfPointsPerQuadrant;
        double refillPeriod @unit(s);
        @display("i=block/table2");
        @class(QuadTreeNeighborCache);
}

