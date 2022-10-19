//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.arp.ipv4;

import inet.networklayer.contract.IArp;

//
// This module provides global address resolution without exchanging packets.
//
simple GlobalArp like IArp
{
    parameters:
        string interfaceTableModule;   // path to the InterfaceTable module
        string addressType @enum("ipv4", "ipv6", "mac", "moduleid", "modulepath") = default("ipv4");
        @display("i=block/layer");
    gates:
        input ifIn @labels(ArpPacket,Ieee802Ctrl);
        output ifOut @labels(ArpPacket,Ieee802Ctrl);
}

