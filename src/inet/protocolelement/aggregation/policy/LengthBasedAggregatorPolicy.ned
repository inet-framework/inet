//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.policy;

import inet.protocolelement.aggregation.contract.IAggregatorPolicy;

//
// Implements an aggregation policy that makes decisions based on packet length
// constraints, controlling when and how packets are combined into aggregates.
//
// Operation:
// - Tracks the number of collected subpackets
// - Monitors the total length of collected subpackets
// - Triggers aggregation when minimum thresholds are reached
// - Enforces maximum limits on aggregated packet size
// - Provides configurable parameters for fine-tuning aggregation behavior
//
// Particularly useful for:
// - Optimizing network efficiency by combining small packets
// - Implementing size-based packet bundling strategies
// - Controlling overhead in aggregation protocols
// - Balancing between latency and throughput in packet processing
// - Creating predictable aggregation behavior based on packet sizes
//
// @see ~IAggregatorPolicy, ~AggregatorBase
//
simple LengthBasedAggregatorPolicy like IAggregatorPolicy
{
    parameters:
        int minNumSubpackets = default(0);
        int maxNumSubpackets;
        int minAggregatedLength @unit(b) = default(0b);
        int maxAggregatedLength @unit(b);
        @display("i=block/star");
        @class(LengthBasedAggregatorPolicy);
}
