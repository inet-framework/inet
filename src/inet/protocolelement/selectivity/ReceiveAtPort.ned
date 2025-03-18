//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

//
// Filters packets based on the destination port. Only packets with a matching
// destination port are passed to the output gate, while other packets are
// dropped.
//
simple ReceiveAtPort extends SimpleModule
{
    parameters:
        @class(ReceiveAtPort);
        int port;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
