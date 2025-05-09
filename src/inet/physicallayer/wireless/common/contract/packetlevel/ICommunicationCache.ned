//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// Module interface for communication caches that store and manage radio and
// transmission data in wireless networks. Provides efficient caching of
// intermediate computation results related to signal propagation, reception,
// and interference to improve simulation performance by avoiding redundant
// calculations.
//
moduleinterface ICommunicationCache
{
    parameters:
        @display("i=block/table2");
}

