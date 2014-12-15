//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.routing.dymo;

import inet.networklayer.multi.NetworkDatagramMultiplexer;

//
// This module interface is used by DYMO implementations.
//
moduleinterface IDYMO
{
    parameters:
        @display("i=block/routing");
    gates:
        input ipIn;
        output ipOut;
}

//
// This module provides Dynamic MANET On-demand (DYMO also known as AODVv2) Routing
// based on the IETF draft at http://tools.ietf.org/html/draft-ietf-manet-dymo-24.
//
// Module parameters are copied from the IETF draft and renamed for consistency with
// INET naming conventions. Some parameters are not yet implemented.
//
simple DYMO like IDYMO
{
    parameters:
        // properties
        @class("dymo::DYMO");
        @display("i=block/routing");

        // context parameters
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string networkProtocolModule = default("^.networkLayer.ip");

        // 1. DYMO parameter group
        string clientAddresses = default("10.10.0.0/24 10.20.0.0"); // CLIENT_ADDRESSES
        bool useMulticastRREP = default(false); // USE_MULTICAST_RREP
        // string DEFAULT_METRIC_TYPE = default("HOP_COUNT");
        string interfaces = default("*"); // AODVv2_INTERFACES

        // 2. DYMO parameter group
        double activeInterval @unit("s") = default(5s); // ACTIVE_INTERVAL
        double maxIdleTime @unit("s") = default(200s); // MAX_IDLETIME
        double maxSequenceNumberLifetime @unit("s") = default(300s); // MAX_SEQNUM_LIFETIME
        double routeRREQWaitTime @unit("s") = default(2s); // ROUTE_RREQ_WAIT_TIME
        // double UNICAST_MESSAGE_SENT_TIMEOUT @unit("s") = default(1s);
        double rreqHolddownTime @unit("s") = default(10s); // RREQ_HOLDDOWN_TIME

        // 3. DYMO parameter group
        int maxHopCount = default(20); // MAX_HOPCOUNT
        // string MAX_METRIC = default("");
        // double MAXTIME;
        int discoveryAttemptsMax = default(3); // DISCOVERY_ATTEMPTS_MAX
        // int MTU;

        // 4. DYMO parameter group
        bool appendInformation = default(true); //APPEND_INFORMATION;
        int bufferSizePackets = default(-1); // BUFFER_SIZE_PACKETS
        int bufferSizeBytes @unit("B") = default(-1B); //  BUFFER_SIZE_BYTES
        // double CONTROL_TRAFFIC_LIMIT

        // DYMO extension parameters
        double maxJitter @unit("s") = default(10ms);
        bool sendIntermediateRREP = default(true);
        int minHopLimit = default(5);
        int maxHopLimit = default(10);

    gates:
        input ipIn;
        output ipOut;
}

//
// This module provides DYMO routing for multiple network protocols simultaneously.
//
module MultiDYMO like IDYMO
{
    parameters:
        @display("i=block/table");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableGeneric = default(true);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;

        *.interfaceTableModule = default(absPath(interfaceTableModule));
        ipv4.routingTableModule = default(absPath(routingTableModule)  + ".ipv4");
        ipv4.networkProtocolModule = default("^.^.networkLayer.ipv4.ip");
        ipv6.routingTableModule = default(absPath(routingTableModule)  + ".ipv6");
        ipv6.networkProtocolModule = default("^.^.networkLayer.ipv6.ipv6");
        generic.routingTableModule = default(absPath(routingTableModule)  + ".generic");
        generic.networkProtocolModule = default("^.^.networkLayer.generic.gnp");

    gates:
        input ipIn;
        output ipOut;

    submodules:
        ipv4: DYMO if enableIPv4 {
            @display("p=100,100");
        }
        ipv6: DYMO if enableIPv6 {
            @display("p=200,100");
        }
        generic: DYMO if enableGeneric {
            @display("p=300,100");
        }
        multiplexer: NetworkDatagramMultiplexer {
            @display("p=200,200");
        }

    connections allowunconnected:
        multiplexer.upperOut --> ipOut;
        ipIn --> multiplexer.upperIn;
        multiplexer.lowerOut++ --> ipv4.ipIn if enableIPv4;
        ipv4.ipOut --> multiplexer.lowerIn++ if enableIPv4;
        multiplexer.lowerOut++ --> ipv6.ipIn if enableIPv6;
        ipv6.ipOut --> multiplexer.lowerIn++ if enableIPv6;
        multiplexer.lowerOut++ --> generic.ipIn if enableGeneric;
        generic.ipOut --> multiplexer.lowerIn++ if enableGeneric;
}
