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
// content using a configurable filter expression. This module enables sophisticated
// traffic filtering based on packet data and metadata, including headers,
// payload, and attached tags.
//
// Operation mechanism:
// - Evaluates each packet against the configured filter expression
// - Forwards packets that match the filter criteria, drops others
// - In push mode: Processes packets from producer, forwards matching ones to consumer
// - In pull mode: Pulls packets from provider until finding a matching one
//
simple ContentBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        object packetFilter = default("*"); // Packet filter expression, matches all packets by default
        @class(ContentBasedFilter);
}
