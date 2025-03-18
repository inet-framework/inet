//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Filters packets based on the destination MAC address. Only packets with a
// matching destination address are passed to the output gate, while other
// packets are dropped. Also sets the MAC address of the containing network
// interface.
//
simple ReceiveAtMacAddress
{
    parameters:
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
