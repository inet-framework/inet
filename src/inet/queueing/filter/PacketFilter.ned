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
// the packet to the connected packet consumer or drops it depending on the
// configured packet filter. Similarly, when the filter is pulled by a connected
// packet collector, it pulls packets from the connected packet provider until
// it finds one that matches the configured packet filter.
//
simple PacketFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string filterClass; // Determines the packets that pass the filter, the parameter must be the name of a C++ class that implements the IPacketFilterFunction interface and is registered via Register_Class
        @class(PacketFilter);
}
