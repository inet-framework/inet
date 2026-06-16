//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.icmpv6;

import inet.common.SimpleModule;

//
// Implements ICMPv6 (RFC 4443): it generates and processes ICMPv6 error
// messages (destination unreachable, packet too big, time exceeded, parameter
// problem) on behalf of ~Ipv6, and answers Echo Requests (ping) with Echo
// Replies. Neighbor Discovery messages (RS/RA/NS/NA/Redirect) are not handled
// here but by ~Ipv6NeighbourDiscovery.
//
// The `transportIn`/`transportOut` gates connect to upper-layer users such as a
// ping application; the `ipv6In`/`ipv6Out` gates connect to ~Ipv6.
//
// @see ~Ipv6, ~Ipv6RoutingTable, ~Ipv6NeighbourDiscovery
//
simple Icmpv6 extends SimpleModule
{
    parameters:
        @class(Icmpv6);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string checksumMode @enum("declared", "computed") = default("declared");
        string mldModule = default("");  // Path to the MLD module, or "" if MLD is not enabled; MLD messages are dropped when empty
        @display("i=block/control");
    gates:
        input transportIn;
        output transportOut;
        input ipv6In;
        output ipv6Out;
}

