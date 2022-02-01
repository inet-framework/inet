//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.communicationcache;

import inet.physicallayer.wireless.common.contract.packetlevel.ICommunicationCache;

//
// This communication cache model allows validating other implementations. It
// can be used to provide a reference for communication logs, simulation
// fingerprints, and statistical results that can be compared with results of
// other communication cache implementations.
//
module ReferenceCommunicationCache like ICommunicationCache
{
    parameters:
        @display("i=block/table2");
        @class(ReferenceCommunicationCache);
}

