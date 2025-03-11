//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Deep packet inspection filter that selectively forwards packets based on their
// content using configurable filter expressions. This module enables sophisticated
// traffic filtering based on packet data and metadata.
//
// Operation mechanism:
// - Evaluates each packet against the configured filter expressions
// - Forwards packets that match the filter criteria, drops others
// - In push mode: Processes packets from producer, forwards matching ones to consumer
// - In pull mode: Pulls packets from provider until finding a matching one
//
// The ContentBasedFilter is particularly useful for:
// - Protocol-specific filtering based on header fields
// - Application-layer content filtering
// - Implementing complex firewall rules
// - Quality of Service packet selection
// - Testing protocol implementations with specific packet patterns
//
// The packetFilter parameter contains filter expressions that determine which
// packets are allowed to pass through. The default value "*" matches all packets.
// Filter expressions can examine any aspect of the packet, including headers,
// payload, and attached tags.
//
// @see ~PacketFilter, ~LabelFilter, ~RedDropper
//
simple ContentBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        object packetFilter = default("*"); // Packet filter expressions, matches all packets by default
        @class(ContentBasedFilter);
}
