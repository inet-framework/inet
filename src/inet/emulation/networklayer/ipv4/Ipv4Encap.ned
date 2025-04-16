//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.common.SimpleModule;

//
// Handles IPv4 encapsulation and decapsulation for network emulation. Adds IPv4
// headers to outgoing packets and removes them from incoming packets. Processes
// packet tags for fields like TOS, DSCP, ECN, and hop limit. Supports socket
// operations and manages protocol registration for dispatching packets to the
// appropriate upper layer protocols.
//
simple Ipv4Encap extends SimpleModule
{
    parameters:
        @class(Ipv4Encap);
        @display("i=block/square");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
