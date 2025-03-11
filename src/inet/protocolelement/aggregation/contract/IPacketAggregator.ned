//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Defines the interface for packet aggregator modules that combine multiple
// packets into a single aggregated packet for more efficient transmission.
//
// Operation:
// - Collects individual packets from upper layers
// - Combines multiple packets into a single aggregated packet
// - Preserves packet boundaries and structure for later deaggregation
// - Applies protocol-specific aggregation rules and headers
// - Forwards aggregated packets to lower layers
//
// Particularly useful for:
// - Reducing protocol overhead in network transmissions
// - Improving efficiency for small packet transmissions
// - Implementing packet bundling in various protocols
// - Supporting frame aggregation in wireless networks
// - Creating protocol-specific aggregation mechanisms
//
// @see ~IPacketDeaggregator, ~AggregatorBase
//
moduleinterface IPacketAggregator
{
    parameters:
        @display("i=block/join");
}
