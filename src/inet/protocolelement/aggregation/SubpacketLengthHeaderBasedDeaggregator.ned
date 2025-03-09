//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.DeaggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketDeaggregator;

//
// Implements a packet deaggregator that extracts individual packets from
// aggregated packets by interpreting length headers that mark subpacket boundaries.
//
// Operation:
// - Receives aggregated packets from lower layers
// - Reads length headers to identify subpacket boundaries
// - Extracts individual subpackets from the aggregated packet
// - Removes length headers from extracted subpackets
// - Forwards extracted packets to upper layers
//
// @see ~SubpacketLengthHeaderBasedAggregator, ~DeaggregatorBase
//
simple SubpacketLengthHeaderBasedDeaggregator extends DeaggregatorBase like IPacketDeaggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedDeaggregator);
}
