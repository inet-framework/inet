//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.communicationcache;

import inet.physicallayer.wireless.common.contract.packetlevel.ICommunicationCache;

//
// This communication cache model stores radio, transmission and reception
// related intermediate computation results in vector data structures. It's
// primarily useful for simulations with <b>either</b> static <b>or</b> short
// lived dynamic radios, and with only short transmission durations.
//
module VectorCommunicationCache like ICommunicationCache
{
    parameters:
        @display("i=block/table2");
        @class(VectorCommunicationCache);
}

