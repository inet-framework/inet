//
// Copyright (C) 2013 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.routing.rip;

import inet.applications.contract.IUDPApp;


//
// Routing Information Protocol.
//
// This module implements distance vector routing as
// specified in RFC 2453 (RIPv2) and RFC 2080 (RIPng).
// The routing protocol uses the Bellman-Ford algorithm to compute
// the optimal routes. Each router periodically sends its
// routes to the neighboring routers. This update message
// contains the destination (address, netmask) and metric
// of each route. When a router receives an update message,
// it adds the cost of the incoming interface
// to the cost of the received route and if it is smaller than the metric
// of its current route then it updates the current route.
//
// Limitations of the protocol:
//   - only the hop-count metric is supported
//   - the metric diameter of the network must be smaller than 16
//   - the protocol depends on "counting to infinity" to recover
//     from routing loops
//
// The module has the following parameters:
//   - mode: either "RIPv2" (RFC 2453) or "RIPng" (RFC 2080)
//   - routingTableModule: path to the routing table module
//   - ripConfig: an XML configuration file containing per-interface parameters
//
// The configuration file specifies the per interface parameters.
// Each <interface> element configures one or more interfaces;
// the @hosts, @names, @towards, @among attributes select the
// configured interfaces (see ~IPv4NetworkConfigurator).
// The other attributes sets parameters of the selected interfaces:
//
// - @metric: metric assigned to the link, default value is 1.
//            This value is added to the metric of a learned route,
//            received on this interface. It must be an integer in
//            the [1,15] interval.
// - @mode: mode of the interface; an enumerated value, default is
//          'SplitHorizonPoisonedReverse'. Accepted values are:
//     - 'NoRIP': no RIP messages are sent or received on this interface
//     - 'NoSplitHorizon': no split horizon filtering; send all routes to
//                         neighbors.
//     - 'SplitHorizon':   do not sent routes whose next hop is the neighbor
//     - 'SplitHorizonPoisenedReverse': if the next hop is the neighbor, then
//                         set the metric of the route to infinity.
simple RIPRouting like IUDPApp
{
    parameters:
        @display("i=block/network2");

        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string mode @enum("RIPv2", "RIPng") = default("RIPv2");
        xml ripConfig = default(xml("<config><interface metric='1'/></config>"));

        int udpPort = (mode == "RIPng" ? 521 : 520);
        double updateInterval @unit(s) = default(30s);
        volatile double startupTime @unit(s) = default(uniform(0s,5s));
        volatile double triggeredUpdateDelay @unit(s) = default(uniform(1s,5s));
        double routeExpiryTime @unit(s) = default(180s);
        double routePurgeTime @unit(s) = default(120s);
        double shutdownTime @unit(s) = default(1s);
        @signal[sentRequest](type=cPacket);
        @signal[sentUpdate](type=cPacket);
        @signal[rcvdResponse](type=cPacket);
        @signal[badResponse](type=cPacket);
        @signal[numRoutes](type=unsigned long);
        @statistic[sentRequest](title="request sent"; source=sentRequest; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sentUpdate](title="update sent"; source=sentUpdate; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[rcvdResponse](title="response received"; source=rcvdResponse; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[badResponse](title="bad response received"; source=badResponse; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[numRoutes](title="number of routes"; source=numRoutes; record=last,vector; interpolationmode=none);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
