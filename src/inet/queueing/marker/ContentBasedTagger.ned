//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketTaggerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches various request tags to packets.
//
simple ContentBasedTagger extends PacketTaggerBase like IPacketMarker
{
    parameters:
        object packetFilter = default("*"); // packet filter expressions, matches all packets by default
        @class(ContentBasedTagger);
}
