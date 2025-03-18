//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.dispatching;

import inet.common.SimpleModule;

//
// Extracts the protocol from a packet by removing the protocol header, and attaches
// protocol tags to the packet. The protocol information is used for dispatching the
// packet to the appropriate protocol module.
//
simple ReceiveWithProtocol extends SimpleModule
{
    parameters:
        @class(ReceiveWithProtocol);
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
