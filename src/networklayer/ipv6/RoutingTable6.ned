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

//
// IPv6 Routing Table and Neighbour Discovery data structures.
// NOTE: This component MUST be named as "routingTable6" inside
// a StandardHost/Router etc. in order to be accessible by the
// ~IPv6 and other modules
//
// The "routingTable" parameter can be specified as XML document describing
// the address and routing properties of all (or a subset of) nodes in
// a network. By default, an empty routingTable element is used, meaning
// that everything will use default configuration. This usually means link-local
// address autoconfiguration and routing by router advertisements. Alternatively,
// FlatNetworkConfigurator6 can be used to setup static routes and the like
// automatically. However, segmented networks with different IPv6 subnets
// are not possible using FlatNetworkConfigurator6.
// By using an XML "routingTable", the following properties can be set up
// manually per node and interface: IP addresses, advertised prefixes
// and static routes.
//
// The format is as follows, see examples/ipv6/manualaddresses for an usage example:
//
// The root node "routingTable" contains a number of <local node="nodeName">
// elements pertaining to every node of the network that should use
// XML route configuration.
// Each <local> element then has any number of <interface name="ethX"> elements,
// one for each interface. The optional boolean parameter "sendAdvertisements" can
// be used to configure whether router advertisements should be sent per interface.
// Default will be used if not specified (hosts do not send RAs, routers do on every
// available interface).
// The interface configuration is split into three optional sections:
// * <inetAddr> elements each containing one IPv6 address this interface should
//   should use. Optional boolean parameter "tentative" defaults to false.
// * One <advPrefixList> element listing all prefixes to be advertised (if the
//   interface is configured to send RAs):
//   - <advPrefix> elements each containing one IPv6 address/prefix notation
//     specifying one prefix that should be advertised.
//     Parameters: "onLink": on-link flag (nodes should use prefix for
//                           on-link detection), boolean
//                 "autonomous": autonomous  (nodes should use prefix for
//                               address autoconfiguration), boolean
//                 "validLifetime": integer (seconds), default: 30 days
//                 "preferredLifetime": integer (seconds), default: 7 days
// * One <routeList> element listing all static routes to be set up:
//   - <route> elements each containing one IPv6 address/prefix notation
//     specifying a network for which this interface should be used for
//     delivery.
//     Parameters: "nextHop": IPv6 address outgoing packets matching this
//                            route should be addressed to. If unspecified,
//                            the route is assumed to be direct.
//                 "metric": integer, optional
//
// @see ~IPv6, ~IPv6NeighbourDiscovery, ~ICMPv6
//
simple RoutingTable6
{
    parameters:
        xml routingTable = default(xml("<routingTable/>"));
        bool isRouter;
        bool forwardMulticast = default(false);
        @display("i=block/table");
}
