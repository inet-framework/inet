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
    "IPNode"; 
              
 
channel standardChannel 
//    delay normal(0.005, 0.00002); 
//    datarate 20000; 
endchannel 

//
// Two IPNodes are connected together over a channel.
//
module Peer2Network 
    parameters:
        ipoutput_delay,
        ipsend_delay,
        localdeliver_delay,
        prerouting_delay,
        routing_delay,
        fragmentation_delay,
        enqueue_delay,
        inputqueue_delay : numeric;
        // nwi_delay : numeric ;
    submodules: 
        tcpClientNode: IPNode; 
            parameters: 
                // Network delay times
                fragmentation_delay = fragmentation_delay,
                ipoutput_delay = ipoutput_delay,
                ipsend_delay = ipsend_delay,
                localdeliver_delay = localdeliver_delay,
                prerouting_delay = prerouting_delay,
                routing_delay = routing_delay,
                enqueue_delay = enqueue_delay,
                inputqueue_delay = inputqueue_delay,
                // nwi_delay = nwi_delay,

                // hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                // network parameters
                numOfPorts = 1, 
                routingFile = "OldNetwork/peer2nw_1.irt",
                numOfProcessors = input(1),
                // transport&app parameters
                // UDP app parameters
                packetTime = 1000 s, 
                packetSize = 1000, 
                //packetTime = input(0.5us), 
                //packetSize = input(1000), 
                nodenr = 1,
                // TCP app parameters
                nodename = "tcp client 1", 
                tcpApplType = "TCPClient",
                local_addr = 1,
                server_addr = 2;
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=49,60;i=pc";
        tcpServerNode: IPNode; 
            parameters: 
                // Network delay times
                fragmentation_delay = fragmentation_delay,
                ipoutput_delay = ipoutput_delay,
                ipsend_delay = ipsend_delay,
                localdeliver_delay = localdeliver_delay,
                prerouting_delay = prerouting_delay,
                routing_delay = routing_delay,
                enqueue_delay = enqueue_delay,
                inputqueue_delay = inputqueue_delay,
                // nwi_delay = nwi_delay,


                // hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                // network parameters
                numOfProcessors = input(1),
                routingFile = "OldNetwork/peer2nw_2.irt",
                numOfPorts = 1, 
                // UDP app parameters
                // transport&app parameters
                //packetTime = input("20s"), 
                //packetSize = input(100), 
                packetTime = 1000s, 
                packetSize = 8000, 
                nodenr = 2,
                // TCP app parameters
                nodename = "TCP server", 
                tcpApplType = "TCPServer",
                local_addr = 2,
                server_addr = 2; // should not matter
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=147,62;i=pc";
    connections nocheck: 
        tcpClientNode.out[0] --> standardChannel --> tcpServerNode.in[0]; 
        tcpClientNode.in[0] <-- standardChannel <-- tcpServerNode.out[0]; 
                                                          
    display: "p=10,10;b=196,124,rect";
endmodule 

// instance of the ProcNetwork1
network peer2Network : Peer2Network 
    parameters:
        fragmentation_delay = 0.2 us,
        ipoutput_delay =  0.5 us,
        ipsend_delay = 0.2 us,
        localdeliver_delay = 0.2 us,
        prerouting_delay = 0.5 us,
        routing_delay = 1 us,
        enqueue_delay = 0.2 us ,
        inputqueue_delay = 1 us;
        // nwi_delay = 0;
endnetwork 

