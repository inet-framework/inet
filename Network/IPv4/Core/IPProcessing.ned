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
    "PreRouting", 
    "Routing", 
    "LocalDeliver", 
    "IPMulticast", 
    "ICMP", 
    "IPSend", 
    "IPFragmentation", 
    "IPTunneling", 
    "IPOutput"; 
                
                
 
//
// Simulates the IP Protocol of a host or router.
//
// author: Jochen Reber
// date: 2.5.00
//
module IPProcessing 
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
        routing_delay : numeric const, 
                                       
        // hook names
        preRoutingHookName : string, 
        routingHookName : string, 
        localDeliverHookName : string, 
        ipSendHookName : string, 
        ipOutputHookName : string, 
                                   
                                   
        // other parameters
        timeToLive : numeric, 
        multicastTimeToLive : numeric, 
        numOfPorts : numeric, 
        numOfTransportProt : numeric const, 
        IPForward : bool, 
        fragmentTimeout : string; 
                                  
    gates: 
        in: ICMPIn; 
        in: transportIn[]; 
        in: queueIn; 
        out: ICMPOut; 
        out: transportOut[]; 
        out: queueOut[]; 
        out: errorOut; 
    submodules: 
        preRouting: PreRouting; 
            parameters: 
                hookName = preRoutingHookName, 
                procdelay = prerouting_delay; 
            display: "p=51,133;i=prot3";
        routing: Routing; 
            parameters: 
                hookName = routingHookName, 
                procdelay = routing_delay, 
                IPForward = IPForward; 
            display: "p=114,129;i=bwgen";
        localDeliver: LocalDeliver; 
            parameters: 
                hookName = localDeliverHookName, 
                procdelay = localdeliver_delay, 
                numOfTransportProt = numOfTransportProt, 
                fragmentTimeout = fragmentTimeout; 
            gatesizes: 
                transportOut[numOfTransportProt]; 
            display: "p=66,51;i=fork";
        multicast: IPMulticast; 
            parameters: 
                procdelay = multicast_delay, 
                IPForward = IPForward; 
            display: "p=122,74;i=bwgen_s";
        ICMP: ICMP; 
            parameters: 
                procdelay = icmp_delay; 
            gatesizes: 
                ipOutputIn[numOfPorts]; 
            display: "p=181,53;i=cogwheel";
        send: IPSend; 
            parameters: 
                hookName = ipSendHookName, 
                procdelay = ipsend_delay,
                timeToLive = timeToLive, 
                multicastTimeToLive = multicastTimeToLive, 
                numOfTransportProt = numOfTransportProt; 
            gatesizes: 
                transportIn[numOfTransportProt]; 
            display: "p=247,59;i=prot1";
        fragmentation: IPFragmentation; 
            parameters: 
                procdelay = fragmentation_delay, 
                numOfPorts = numOfPorts; 
            gatesizes: 
                outputOut[numOfPorts]; 
            display: "p=188,131;i=bwxconn";
        tunneling: IPTunneling; 
            parameters: 
                procdelay = iptunneling_delay; 
            display: "p=250,110;i=bwxcon_s";
        output: IPOutput[numOfPorts]; 
            parameters: 
                hookName = ipOutputHookName, 
                procdelay = ipoutput_delay; 
            display: "p=54,218,row;i=prot2";
    connections nocheck: 
        // PreRouting Input
        queueIn --> preRouting.queueIn; 
                                        
        // PreRouting Output
        preRouting.routingOut --> routing.preRoutingIn; 
        preRouting.errorOut --> ICMP.preRoutingIn; 
                                                   
        // Routing output
        routing.localOut --> localDeliver.routingIn; 
        routing.fragmentationOut --> fragmentation.routingIn; 
        routing.multicastOut --> multicast.routingIn; 
        routing.errorOut --> ICMP.routingIn; 
                                             
        // LocalDeliver output
        localDeliver.multicastOut --> multicast.localIn; 
        localDeliver.ICMPOut --> ICMP.localIn; 
        localDeliver.preRoutingOut --> preRouting.localDeliverIn; 
        for i=0..numOfTransportProt-1 do 
            localDeliver.transportOut[i] --> transportOut[i]; 
        endfor; 

        // Multicast output
        multicast.localOut --> localDeliver.multicastIn; 
        multicast.fragmentationOut --> fragmentation.multicastIn; 
        multicast.tunnelOut --> tunneling.multicastIn; 
                                                       
        // ICMP output, ping input
        ICMP.sendOut --> send.ICMPIn; 
        ICMP.pingOut --> ICMPOut; 
        ICMPIn --> ICMP.pingIn; 
        ICMP.errorOut --> errorOut; 
                                    
        // IP send output, input
        send.routingOut --> routing.sendIn; 
        for i=0..numOfTransportProt-1 do 
            transportIn[i] --> send.transportIn[i]; 
        endfor; 

        // IP Fragmentation output 
        fragmentation.errorOut --> ICMP.fragmentationIn; 
                                                         
        // IP Tunnel output
        tunneling.sendOut --> send.tunnelIn; 
                                             
        // Output ports
        for i=0..numOfPorts-1 do 
            fragmentation.outputOut[i] --> output[i].fragmentationIn; 
            output[i].queueOut --> queueOut[i]; 
            output[i].errorOut --> ICMP.ipOutputIn[i]; 
        endfor; 

    display: "p=10,10;b=484,238,rect";
endmodule 

