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
// The AODV is designed for mobile ad hoc networks with populations
// of tens to thousands of mobile nodes. AODV has been designed to
// reduce the dissemination of control traffic and eliminate overhead
// on data traffic, in order to improve scalability and performance.
//
// This routing protocol communicates over UDP and can be
// used in ~AdhocHost nodes as a routing submodule.
//
simple AODVRouting
{
    parameters:
        @display("i=block/network2");
        string routingTablePath = default("^.routingTable");
        string interfaceTablePath = default("^.interfaceTable");
        string networkProtocolPath = default("^.networkLayer.ip");
        bool askGratuitousRREP = default(false);
        int UDPPort = default(654);
    gates:
        input ipIn;
        output ipOut;
}
