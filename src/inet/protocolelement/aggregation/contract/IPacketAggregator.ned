//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Interface for packet aggregator modules that combine multiple packets into a
// single aggregated packet. Aggregation is a technique to improve network
// efficiency by reducing the overhead associated with transmitting many small
// packets. Implementations of this interface collect packets and combine them
// according to specific aggregation policies and protocols.
//
moduleinterface IPacketAggregator
{
    parameters:
        @display("i=block/join");
}
