//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.base;

import inet.protocolelement.aggregation.contract.IAggregatorPolicy;
import inet.queueing.base.PacketPusherBase;

//
// Base module for packet aggregators that provides common functionality for
// combining multiple packets into a single aggregated packet. Uses a pluggable
// aggregation policy to determine when and how to aggregate packets.
//
// Operation:
// - Collects packets from upper layers
// - Consults the aggregation policy to determine aggregation timing
// - Combines multiple packets into a single aggregated packet
// - Adds necessary information to identify subpacket boundaries
// - Forwards aggregated packets to lower layers
//
// Particularly useful for:
// - Creating protocol-specific aggregators with shared base functionality
// - Supporting different aggregation strategies through policy modules
// - Implementing packet bundling for efficiency
// - Building protocol stacks with aggregation capabilities
// - Providing a foundation for specialized aggregator implementations
//
// @see ~DeaggregatorBase, ~IAggregatorPolicy
//
module AggregatorBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        string aggregatorPolicyClass = default("");
        string aggregatorPolicyModule = default(".aggregatorPolicy");
        @display("i=block/join");
        @class(AggregatorBase);
    submodules:
        aggregatorPolicy: <default("LengthBasedAggregatorPolicy")> like IAggregatorPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
