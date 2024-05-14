//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.ipv6tunneling;

import inet.networklayer.contract.IIpv6Tunneling;

//
// IPv6 Tunnel Manager
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6Tunneling like IIpv6Tunneling
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;  // The path to the RoutingTable module
        string xmipv6Module;  // The path to the XMipv6 module
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input linkLayerIn;
        output linkLayerOut;
}

