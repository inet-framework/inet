//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv6;

import inet.common.SimpleModule;
import inet.networklayer.contract.IRoutingTable;

//
// IPv6 Routing Table and Neighbor Discovery data structures.
// NOTE: This component MUST be named as `routingTable6` inside
// a ~StandardHost/~Router etc. in order to be accessible by the
// ~Ipv6 and other modules
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6RoutingTable extends SimpleModule like IRoutingTable
{
    parameters:
        @class(Ipv6RoutingTable);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string ipv6TunnelingModule = default("^.ipv6.iptunneling");
        @display("i=block/table");
        xml routes = default(xml("<routingTable/>"));
        bool forwarding = default(false);
        bool isRouter = default(this.forwarding);
        bool multicastForwarding = default(false);
        bool useAdminDist = default(false);     // Use Cisco-like administrative distances
        @signal[routeAdded](type=inet::Ipv6Route);
        @signal[routeChanged](type=inet::Ipv6Route);
        @signal[routeDeleted](type=inet::Ipv6Route);
        @signal[mrouteAdded](type=inet::IPv6MulticastRoute);
        @signal[mrouteDeleted](type=inet::IPv6MulticastRoute);
        @signal[mrouteChanged](type=inet::IPv6MulticastRoute);
}

