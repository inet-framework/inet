//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.networklayer.manetrouting.aodv;

import inet.applications.IUDPApp;


//
// Ad hoc On-Demand Distance Vector Routing Protocol module.
//
// The Ad hoc On-Demand Distance Vector (AODV) routing protocol is
// intended for use by mobile nodes in an ad hoc network. It offers
// quick adaptation to dynamic link conditions, low processing and
// memory overhead, low network utilization, and determines unicast
// routes to destinations within the ad hoc network.
//
// This routing protocol communicates over UDP and can be
// used in ~AdhocHost nodes as a routing submodule.
//
// This implementation is based on RFC 3561. For more information, you may
// refer to the following link: https://tools.ietf.org/html/rfc3561.html.
//
simple AODVRouting
{
    parameters:
        @display("i=block/network2");
        string routingTablePath = default("^.routingTable");
        string interfaceTablePath = default("^.interfaceTable");
        string networkProtocolPath = default("^.networkLayer.ip");

        // AODV Parameters
        bool askGratuitousRREP = default(false);
        bool useHelloMessages = default(false);
        int udpPort = default(654);
        double maxJitter @unit("s") = default(10ms);
        double activeRouteTimeout @unit("s") = default(3s);
    gates:
        input ipIn;
        output ipOut;
}
