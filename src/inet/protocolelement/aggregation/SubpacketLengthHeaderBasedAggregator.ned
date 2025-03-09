//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.AggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketAggregator;

//
// Implements a packet aggregator that uses length headers to mark subpacket
// boundaries within aggregated packets, enabling efficient deaggregation.
//
// Operation:
// - Collects packets for aggregation according to policy
// - Prepends each subpacket with a length header
// - Combines multiple subpackets into a single aggregated packet
// - Preserves packet boundaries through explicit length information
// - Forwards aggregated packets to lower layers
//
// Particularly useful for:
// - Creating self-describing aggregated packets
// - Supporting efficient deaggregation without additional context
// - Implementing protocol-independent packet bundling
// - Building aggregation systems with explicit boundary markers
// - Enabling dynamic subpacket extraction at the receiver
//
// @see ~SubpacketLengthHeaderBasedDeaggregator, ~AggregatorBase
//
module SubpacketLengthHeaderBasedAggregator extends AggregatorBase like IPacketAggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedAggregator);
}
