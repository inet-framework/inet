//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketLabelerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches various request labels to packets.
//
simple ContentBasedLabeler extends PacketLabelerBase like IPacketMarker
{
    parameters:
        object packetFilters = default(["*"]); // array of packet filter expressions, matches all packets by default
        @class(ContentBasedLabeler);
}
