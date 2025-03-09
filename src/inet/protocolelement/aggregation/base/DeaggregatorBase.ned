//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.base;

import inet.queueing.base.PacketPusherBase;

//
// Base module for packet deaggregators that provides common functionality for
// extracting multiple original packets from a single aggregated packet.
//
// Operation:
// - Receives aggregated packets from lower layers
// - Identifies subpacket boundaries within the aggregated packet
// - Extracts individual packets from the aggregated packet
// - Restores original packet structure and headers
// - Forwards extracted packets to upper layers
//
// Particularly useful for:
// - Creating protocol-specific deaggregators with shared base functionality
// - Implementing the receiving end of packet bundling systems
// - Building protocol stacks with deaggregation capabilities
// - Providing a foundation for specialized deaggregator implementations
// - Complementing aggregator modules in network protocols
//
// @see ~AggregatorBase, ~IPacketDeaggregator
//
simple DeaggregatorBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        @display("i=block/fork");
        @class(DeaggregatorBase);
}
