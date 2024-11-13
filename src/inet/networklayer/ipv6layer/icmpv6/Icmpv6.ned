//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv6layer.icmpv6;

//
// ICMPv6 implementation.
//
// @see ~Ipv6, ~Ipv6RoutingTable, ~Ipv6NeighbourDiscovery
//#FIXME gate descriptions needed...
//
simple Icmpv6
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string crcMode @enum("declared","computed") = default("declared");
        @display("i=block/control");
        @formerly(inet.networklayer.icmpv6.Icmpv6);
    gates:
        input transportIn;
        output transportOut;
        input ipv6In;
        output ipv6Out;
}

