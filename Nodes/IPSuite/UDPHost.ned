// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


import
    "NetworkLayers",
    //"ProcessorManager",
    "UDP",
    "tcp4ip",
    "PingApp",
    "ErrorHandling",
    "TrafficGen";



//
// Definition of an IPNode with transport layers.
//
module UDPHost
    parameters:

        // other application information
        packetSize : numeric const, // (bit)
        packetTime : numeric const,
        nodename : string,
        nodenr : numeric const,
        udpRouteDestNo : numeric const,

        // Network information
        IPForward : bool,
        numOfProcessors : numeric,
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        udpApp: UDPUpperLayers;
            parameters:
                nodename = nodename,
                udpCl1Name = "UDPClient",
                udpServer1Name = "UDPServer",
                generationTime = packetTime,
                generationSize = packetSize,
                routeDestNo = udpRouteDestNo;
            display: "p=89,68;b=40,24,rect";
        errorHandling: ErrorHandling;
            parameters:
                nodename = nodename;
            display: "p=179,158;i=sink";
        // control module
        networkLayers: NetworkLayers;
            parameters:
                // processing delay times
                ipoutput_delay = 0.2 us,
                fragmentation_delay = 0.1 us,
                ipsend_delay = 0.5 us,
                localdeliver_delay = 1 us,
                prerouting_delay = 0,
                routing_delay = 0.2 us,
                icmp_delay = 0,
                iptunneling_delay = 0,
                multicast_delay = 0,

                enqueue_delay = 0.3 us,
                dequeue_delay = 0,
                inputqueue_delay = 0.1 us,
                nwi_delay = 0,


                // hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                // other Network parameters
                IPForward = IPForward,
                NWIName = "PPPModule",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
        //# processorManager: ProcessorManager;
    connections nocheck:
                         // transport connections

        networkLayers.UDPOut --> udpApp.from_ip;
        networkLayers.UDPIn <-- udpApp.to_ip;

        networkLayers.errorOut --> errorHandling.in;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

