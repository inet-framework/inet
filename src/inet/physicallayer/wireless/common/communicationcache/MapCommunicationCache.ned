//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.communicationcache;

import inet.physicallayer.wireless.common.contract.packetlevel.ICommunicationCache;

//
// This communication cache model stores radio, transmission and reception
// related intermediate computation results in map data structures. It's primarily
// useful for simulations with both static <b>and</b> dynamic radios, and with both
// short <b>and</b> long transmission durations.
//
module MapCommunicationCache like ICommunicationCache
{
    parameters:
        @display("i=block/table2");
        @class(MapCommunicationCache);
}

