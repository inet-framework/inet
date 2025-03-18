//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Filters packets based on the destination port. Only packets with a matching
// destination port are passed to the output gate, while other packets are
// dropped.
//
simple ReceiveAtPort
{
    parameters:
        int port;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
