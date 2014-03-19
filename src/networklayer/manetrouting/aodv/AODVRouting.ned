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
        bool askGratuitousRREP = default(false); // please, see the RFC 3561: 6.6.3
        bool useHelloMessages = default(false); // please, see the RFC 3561: 6.9
        int udpPort = default(654);
        double maxJitter @unit("s") = default(10ms); // the delay when we broadcast a control packet
        double helloInterval @unit("s") = default(1s); // every helloInterval seconds a node broadcasts Hello messages (if it is necessary)
        int allowedHelloLoss = default(2); // allowedHelloLoss * helloInterval is the lifetime value for Hello messages
        double activeRouteTimeout @unit("s") = default(3s); // the timeout value for cached routes
        int netDiameter = default(35); // the maximum possible number of hops between two nodes in the network
        double nodeTraversalTime @unit("s") = default(0.04s); // an estimation of the average one-hop traversal time
        int rerrRatelimit = default(10); // maximum number of RERR messages that the AODV may originate
        int rreqRetries = default(2); // specifies the number of times AODV will repeat an expanded ring search for a destination
        int rreqRatelimit = default(10); // maximum number of RREQ messages that the AODV may originate
        int timeoutBuffer = default(2); // plus time to wait for a delayed RREP (due to congestion) (to omit this buffer set it to 0)
        int ttlStart = default(1); // specifies the TTL value when initiating a route request
        int ttlIncrement = default(2); // specifies the value by which the TTL will be incremented each time a RREQ is retransmitted
        int ttlThreshold = default(7); // the maximum value of TTL over which NET_DIAMETER value will be used to broadcast any RREQ
        int localAddTTL = default(2); // it is used by the formula which calculates the initial TTL of the RREQ for a local repair
    gates:
        input ipIn;
        output ipOut;
}
