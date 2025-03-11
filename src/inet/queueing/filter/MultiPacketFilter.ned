//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.contract.IPacketFilter;

//
// Composite packet filter that chains multiple filters in series to create
// complex filtering behavior. This module enables the construction of
// sophisticated packet processing pipelines by combining simpler filters.
//
// Operation mechanism:
// - Arranges multiple packet filters in a sequential chain
// - Each packet must pass through all filters in the chain to be forwarded
// - Packets are dropped if rejected by any filter in the chain
// - The order of filters matters, as each filter processes only packets
//   that passed all previous filters
//
// The MultiPacketFilter is particularly useful for:
// - Implementing multi-stage filtering policies
// - Combining different filtering criteria (e.g., rate limiting + content filtering)
// - Creating complex traffic management solutions
// - Layering security and QoS policies
// - Building modular and reusable filtering configurations
//
// The numFilters parameter determines how many filters are in the chain,
// while the filter[] submodule vector allows specifying different filter types
// for each position in the chain.
//
// @see ~PacketFilter, ~ContentBasedFilter, ~LabelFilter, ~RedDropper
//
module MultiPacketFilter like IPacketFilter
{
    parameters:
        int numFilters;
        @display("i=block/filter");
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
    submodules:
        filter[numFilters]: <> like IPacketFilter {
            @display("p=150,150,row,150");
        }
    connections:
        in --> { @display("m=w"); } --> filter[0].in;
        for i=1..numFilters-1 {
            filter[i - 1].out --> filter[i].in;
        }
        filter[numFilters - 1].out --> { @display("m=e"); } --> out;
}
