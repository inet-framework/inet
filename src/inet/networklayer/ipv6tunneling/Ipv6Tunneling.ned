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
        string routingTableModule;
        string xmipv6Module;
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input linkLayerIn;
        output linkLayerOut;
}

