//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Defines the interface for packet deaggregator modules that extract multiple
// original packets from a single aggregated packet received from lower layers.
//
// Operation:
// - Receives aggregated packets from lower layers
// - Identifies subpacket boundaries within the aggregated packet
// - Extracts individual packets from the aggregated packet
// - Restores original packet structure and headers
// - Forwards extracted packets to upper layers
//
// @see ~IPacketAggregator, ~DeaggregatorBase
//
moduleinterface IPacketDeaggregator
{
    parameters:
        @display("i=block/fork");
}
