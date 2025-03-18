//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.forwarding;

import inet.common.SimpleModule;

//
// Processes incoming packets with hop limit headers. Filters packets based on
// their hop limit value, discarding packets with expired hop limits. Extracts
// the hop limit information and attaches it as a tag to the packet.
//
simple ReceiveWithHopLimit extends SimpleModule
{
    parameters:
        @class(ReceiveWithHopLimit);
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
