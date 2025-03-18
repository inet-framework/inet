//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

//
// Filters packets based on the destination MAC address. Only packets with a
// matching destination address are passed to the output gate, while other
// packets are dropped. Also sets the MAC address of the containing network
// interface.
//
simple ReceiveAtMacAddress extends SimpleModule
{
    parameters:
        @class(ReceiveAtMacAddress);
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
