//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.contract;

//
// IPv6 Tunnel Manager
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
moduleinterface IIpv6Tunneling
{
    @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input linkLayerIn;
        output linkLayerOut;
}

