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

package inet.networklayer.routing.rip;

//
// Routing Information Protocol v2 (RFC 2453).
//
// distance vector routing
//
// limitations:
//   the metric diameter of the network must be smaller than 16
//
simple RIPRouting
{
    parameters:
        string routingTableModule = default("^.routingTable");
        xml ripConfig = default(xml("<config><interface metric='1'/></config>"));

        double updateInterval @unit(s) = 30s;
        volatile double triggeredUpdateDelay @unit(s) = uniform(1s,5s);
        double routeExpiryTime @unit(s) = 180s;
        double routePurgeTime @unit(s) = 120s;
        @signal[sentRequest](type=cPacket);
        @signal[sentUpdate](type=cPacket);
        @signal[rcvdResponse](type=cPacket);
        @signal[badResponse](type=cPacket);
        @statistic[sentRequest](title="request sent"; source=sentRequest; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sentUpdate](title="update sent"; source=sentUpdate; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[rcvdResponse](title="response received"; source=rcvdResponse; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[badResponse](title="bad response received"; source=badResponse; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[numRoutes](title="number of routes"; source=numRoutes; record=last,vector; interpolationmode=none);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
