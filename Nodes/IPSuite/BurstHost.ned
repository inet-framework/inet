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
//



import
    "NetworkLayers",
    //"ProcessorManager",
    "PingApp",
    "ErrorHandling",
    "TrafficGen";



//
// Definition of an IPNode with transport layers.
//
module BurstHost
    parameters:

        nodenr          : numeric const,
        burstPackets    : numeric const,
        packetSize      : numeric const,
        nodename        : string,

        // Network information
        routingFile     : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        burstGenerator: TrafficGenerator
            parameters:
                GeneratorType = "BurstGenerator",
                nodename = nodename,
                nodenr = nodenr,
                tcpProtocol = false,
                burstPackets = burstPackets,
                generationTime = 0,
                generationSize = packetSize;
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
                IPForward = false,
                NWIName = "PPPModule",
                numOfPorts = 1,
                routingFile = routingFile;
            gatesizes:
                physIn[1],
                physOut[1];
            display: "p=87,155;i=fork";
        //# processorManager: ProcessorManager;
    connections nocheck:

        // transport connections

        networkLayers.UDPOut --> burstGenerator.in;
        networkLayers.UDPIn <-- burstGenerator.out;

        networkLayers.errorOut --> errorHandling.in;

        // connections to network outside
        for i=0..0 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

