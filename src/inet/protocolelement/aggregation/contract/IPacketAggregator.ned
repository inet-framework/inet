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
// @see ~IPacketDeaggregator, ~AggregatorBase
//
moduleinterface IPacketAggregator
{
    parameters:
        @display("i=block/join");
}
