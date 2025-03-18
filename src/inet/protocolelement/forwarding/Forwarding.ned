//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.forwarding;

import inet.common.SimpleModule;

//
// Forwards packets based on their destination address. Determines the next hop
// address and interface for outgoing packets, and attaches the appropriate
// address and interface tags to the packet.
//
simple Forwarding extends SimpleModule
{
    parameters:
        @class(Forwarding);
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
