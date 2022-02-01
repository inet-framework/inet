//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.contract.IPacketFilter;

//
// This module combines multiple packet filters into one.
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
