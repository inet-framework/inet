//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.dispatching;

import inet.common.SimpleModule;

//
// Prepends a protocol header to outgoing packets based on the protocol tag
// attached to the packet. Registers the protocol with the appropriate gates to
// enable protocol-based dispatching.
//
simple SendWithProtocol extends SimpleModule
{
    parameters:
        @class(SendWithProtocol);
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
