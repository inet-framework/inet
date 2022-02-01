//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The neighbor cache model computes the affected set of receivers on the medium
// for a given transmission.
//
moduleinterface INeighborCache
{
    parameters:
        double refillPeriod @unit(s);
        @display("i=block/table2");
}

