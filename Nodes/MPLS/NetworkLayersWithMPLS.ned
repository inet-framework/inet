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
    "IPProcessing", 
    "RoutingTable", 
    "InputQueue", 
    "OutputQueue", 
    "NetworkInterface"; 
                        
                        
//
// A version of NetworkLayers, with additional gates for OSPF and RSVP-TE
// 
module NetworkLayersWithMPLS
    parameters: 
                // processing delay times
        icmp_delay : numeric, 
        fragmentation_delay : numeric, 
        multicast_delay : numeric, 
        ipoutput_delay : numeric, 
        ipsend_delay : numeric, 
        iptunneling_delay : numeric, 
        localdeliver_delay : numeric, 
        prerouting_delay : numeric, 
        routing_delay : numeric, 
        enqueue_delay : numeric, 
        dequeue_delay : numeric, 
        inputqueue_delay : numeric, 
        nwi_delay : numeric const, 
                                   
                                   // hook Names
        preRoutingHookName : string, 
        routingHookName : string, 
        localDeliverHookName : string, 
        ipSendHookName : string, 
        ipOutputHookName : string, 
        enqueueHookName : string, 
        dequeueHookName : string, 
                                  
        // other network parameters
        IPForward : bool, 
        NWIName : string, 
        numOfPorts : numeric, 
        routingFile : string; 
    gates: 
        in: physIn[]; 
        in: TCPIn; 
        in: UDPIn; 
        in: OSPFIn; 
        in: RSVPIn; 
        in: pingIn; 
        out: physOut[]; 
        out: TCPOut; 
        out: UDPOut; 
        out: OSPFOut; 
        out: RSVPOut; 
        out: pingOut; 
        out: errorOut; 
    // Submodules:
    submodules: 
        proc: IPProcessing; 
            parameters: 
                // delay times
                icmp_delay = icmp_delay, 
                fragmentation_delay = fragmentation_delay, 
                multicast_delay = multicast_delay, 
                ipoutput_delay = ipoutput_delay, 
                ipsend_delay = ipsend_delay, 
                iptunneling_delay = iptunneling_delay, 
                localdeliver_delay = localdeliver_delay, 
                prerouting_delay = prerouting_delay, 
                routing_delay = routing_delay, 
                                               
                // hook names
                preRoutingHookName = preRoutingHookName, 
                routingHookName = routingHookName, 
                localDeliverHookName = localDeliverHookName, 
                ipSendHookName = ipSendHookName, 
                ipOutputHookName = ipOutputHookName, 
                                                     
                                                     // other parameters
                IPForward = IPForward, 
                numOfPorts = numOfPorts, 
                numOfTransportProt = 4,
                timeToLive = 8, 
                multicastTimeToLive = 6, 
                fragmentTimeout = "5s"; 
            gatesizes: 
                transportIn[4], 
                transportOut[4], 
                queueOut[numOfPorts]; 
            display: "b=32,30;p=172,66;i=cogwheel";
        outputQueue: OutputQueue[numOfPorts]; 
            parameters: 
                enqueueHookName = enqueueHookName, 
                dequeueHookName = dequeueHookName, 
                enqueue_delay = enqueue_delay, 
                dequeue_delay = dequeue_delay; 
            display: "b=32,30;p=130,127;i=queue";
        inputQueue: InputQueue; 
            parameters: 
                procdelay = inputqueue_delay; 
            gatesizes: 
                fromNW[numOfPorts]; 
            display: "p=39,53;i=queue";
        networkInterface: NWIName[numOfPorts] like NetworkInterface; 
            parameters: 
                procdelay = nwi_delay; 
            display: "p=55,181;i=bwxconn";
        routingTable: RoutingTable; 
            parameters: 
                IPForward = IPForward, 
                routingTableFileName = routingFile; 
            display: "p=238,64;i=box3";
    // Connections:
    connections nocheck: 
        // transport Layer
        proc.transportOut[0] --> TCPOut; 
        proc.transportIn[0] <-- TCPIn; 
                                       
        proc.transportOut[1] --> UDPOut; 
        proc.transportIn[1] <-- UDPIn; 
                                       
        proc.transportOut[2] --> OSPFOut; 
        proc.transportIn[2] <-- OSPFIn; 
                                        
        proc.transportOut[3] --> RSVPOut; 
        proc.transportIn[3] <-- RSVPIn; 
                                        
        // ICMP interface
        proc.ICMPOut --> pingOut; 
        proc.ICMPIn <-- pingIn; 
                                
        proc.errorOut --> errorOut; 
                                    
        // L2 IP Queues to IP
        inputQueue.toIP --> proc.queueIn; 
        for i=0..numOfPorts-1 do 
            proc.queueOut[i] --> outputQueue[i].fromIP; 
        endfor; 

        // Network interfaces to L2 IP Queue
        for i=0..numOfPorts-1 do 
            networkInterface[i].ipInputQueueOut --> inputQueue.fromNW[i]; 
            networkInterface[i].ipOutputQueueOut --> outputQueue[i].fromNW; 
            networkInterface[i].ipOutputQueueIn <-- outputQueue[i].toNW; 
        endfor; 

        // Network interfaces to physical layer
        for i=0..numOfPorts-1 do 
            networkInterface[i].physicalOut --> physOut[i]; 
            networkInterface[i].physicalIn <-- physIn[i]; 
        endfor; 

    display: "p=10,10;b=541,211,rect";
endmodule 

