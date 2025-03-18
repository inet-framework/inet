//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

//
// Filters packets based on the destination network layer address. Only packets
// with a matching destination address are passed to the output gate, while
// other packets are dropped.
//
simple ReceiveAtL3Address extends SimpleModule
{
    parameters:
        @class(ReceiveAtL3Address);
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
