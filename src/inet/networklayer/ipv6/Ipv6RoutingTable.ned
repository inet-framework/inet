//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.ipv6;

import inet.networklayer.contract.IRoutingTable;

//
// IPv6 Routing Table and Neighbour Discovery data structures.
// NOTE: This component MUST be named as "routingTable6" inside
// a ~StandardHost/~Router etc. in order to be accessible by the
// ~Ipv6 and other modules
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6RoutingTable like IRoutingTable
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string ipv6TunnelingModule = default("^.ipv6.iptunneling");
        @display("i=block/table");
        xml routes = default(xml("<routingTable/>"));
        bool forwarding = default(false);
        bool isRouter = default(this.forwarding);
        bool multicastForwarding = default(false);
        bool useAdminDist = default(false);     // Use Cisco like administrative distances
        @signal[routeAdded](type=inet::Ipv6Route);
        @signal[routeChanged](type=inet::Ipv6Route);
        @signal[routeDeleted](type=inet::Ipv6Route);
        @signal[mrouteAdded](type=inet::IPv6MulticastRoute);
        @signal[mrouteDeleted](type=inet::IPv6MulticastRoute);
        @signal[mrouteChanged](type=inet::IPv6MulticastRoute);
}

