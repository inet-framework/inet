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
    "ProcessorManager", 
    "UDP",
    "tcp4ip",
    "PingApp", 
    "ErrorHandling"; 
 

//
// Definition of an IPNode with transport layers.
//
module StandardHost
    parameters: 
        // TCP information
        tcpApplType : string,
        local_addr : numeric,
        server_addr : numeric,

        // UDP and other application information
        packetSize : numeric const,  // (bit)
        packetTime : numeric const,
        nodename : string,
        nodenr : numeric const,
        udpRouteDestNo : numeric const,

        // Network information  
        numOfProcessors : numeric,
        numOfPorts : numeric, 
        routingFile : string; 
    gates: 
        in: in[]; 
        out: out[]; 
    submodules: 
        // transport and application layers
        tcpApp: TCPUpperLayers;
            parameters:
                tcpApplType = tcpApplType,
                local_addr = local_addr,
                server_addr = server_addr;
            display: "p=36,93;b=40,24,rect";
        udpApp: UDPUpperLayers;
            parameters:
                nodename = nodename,
                udpCl1Name = "UDPClient",
                udpServer1Name = "UDPServer",
                generationTime = packetTime,
                generationSize = packetSize,
                routeDestNo = udpRouteDestNo;
            display: "p=97,92;b=40,24,rect";
        pingApp: PingApp;
            parameters:
                nodename = nodename,
                generationTime = packetTime;
            display: "p=152,89;b=40,24,rect";
        errorHandling: ErrorHandling; 
            parameters: 
                nodename = routingFile; 
            display: "p=179,158;i=sink";
        networkLayers: NetworkLayers; 
            parameters: 
                // waiting times
                icmp_delay = 0,
                fragmentation_delay = 0.2 us,
                multicast_delay = 0.5 us,
                ipoutput_delay = 0,
                ipsend_delay = 0,
                iptunneling_delay = 0,
                localdeliver_delay = 0,
                prerouting_delay = 0.2 us,
                routing_delay = 1 us,
                enqueue_delay = 0,
                dequeue_delay = 0,
                inputqueue_delay = 0,
                nwi_delay = 0,
                
                // hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                // other network parameters
                IPForward = false,
                NWIName = "PPPModule", 
                numOfPorts = numOfPorts, 
                routingFile = routingFile; 
            gatesizes: 
                physIn[numOfPorts], 
                physOut[numOfPorts]; 
            display: "p=87,155;i=fork";
        processorManager: ProcessorManager; 
            parameters: 
                numOfProcessors = numOfProcessors; 
            display: "p=165,43;i=bwgen";
    connections nocheck: 
        // transport connections
        networkLayers.TCPOut --> tcpApp.in; 
        networkLayers.TCPIn <-- tcpApp.out; 

        networkLayers.UDPOut --> udpApp.from_ip; 
        networkLayers.UDPIn <-- udpApp.to_ip; 
                                            

        networkLayers.pingOut --> pingApp.in;
        networkLayers.pingIn <-- pingApp.out;

        networkLayers.errorOut --> errorHandling.in; 
                                                     
        // connections to network outside
        for i=0..numOfPorts-1 do 
            in[i] --> networkLayers.physIn[i]; 
            out[i] <-- networkLayers.physOut[i]; 
        endfor; 

endmodule 

