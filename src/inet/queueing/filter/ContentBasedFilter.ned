//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters packets that match the configured filter criteria.
// When the filter is pushed by a connected packet producer, it either pushes
// the packet to the connected packet consumer or drops it, depending on the
// configured packet filter. Similarly, when the filter is pulled by a connected
// packet collector, it pulls packets from the connected packet provider until
// it finds one that matches the configured packet filter.
//
simple ContentBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        object packetFilter = default("*"); // Packet filter expressions, matches all packets by default
        @class(ContentBasedFilter);
}
