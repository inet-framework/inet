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
        timeToLive : numeric, 
        multicastTimeToLive : numeric, 
        numOfPorts : numeric, 
        numOfTransportProt : numeric const, 
        IPForward : bool, 
        fragmentTimeout : string; 
                                  
    gates: 
        in: ICMPIn; // FIXME should be called pingIn
        in: transportIn[]; 
        in: queueIn; 
        out: ICMPOut; // FIXME should be called pingOut
        out: transportOut[]; 
        out: queueOut[]; 
        out: errorOut; 
    submodules: 
        preRouting: PreRouting; 
            gatesizes: 
                in[2]; 
            display: "b=32,30;p=95,177;i=prot3";
        routing: Routing; 
            parameters: 
                IPForward = IPForward; 
            gatesizes: 
                in[2]; 
            display: "b=34,34;p=179,177;i=bwgen";
        localDeliver: LocalDeliver; 
            parameters: 
                fragmentTimeout = fragmentTimeout; 
            gatesizes: 
                in[2], 
                transportOut[numOfTransportProt]; 
            display: "b=32,30;p=95,78;i=fork";
        multicast: IPMulticast; 
            parameters: 
                IPForward = IPForward; 
            display: "b=17,17;p=305,175;i=bwgen_s";
        icmp: ICMP; 
            display: "b=32,30;p=259,78;i=cogwheel";
        send: IPSend; 
            parameters: 
                timeToLive = timeToLive, 
                multicastTimeToLive = multicastTimeToLive; 
            gatesizes: 
                in[numOfTransportProt+2]; 
            display: "b=32,30;p=372,78;i=prot1";
        fragmentation: IPFragmentation; 
            parameters: 
                numOfPorts = numOfPorts; 
            gatesizes: 
                outputOut[numOfPorts]; 
            display: "b=32,30;p=231,249;i=bwxconn";
        tunneling: IPTunneling; 
            display: "b=16,15;p=370,152;i=bwxcon_s";
        output: IPOutput[numOfPorts]; 
            display: "b=32,30;p=110,304,row;i=prot2";
        errorHandling: ErrorHandling; 
            parameters: 
                nodename = "<nodename>"; //FIXME "ancestor nodename" is not good
            display: "b=16,16;p=313,40;i=box1_s";
    connections nocheck: 
        // PreRouting Input
        queueIn --> preRouting.in[0]; 
                                      
        // PreRouting Output
        preRouting.routingOut --> routing.in[0]; 
                                                 
        // Routing output
        routing.localOut --> localDeliver.in[0]; 
        routing.fragmentationOut --> fragmentation.routingIn; 
        routing.multicastOut --> multicast.routingIn; 
                                                      
        // LocalDeliver output
        localDeliver.multicastOut --> multicast.localIn; 
        localDeliver.ICMPOut --> icmp.localIn; 
        localDeliver.preRoutingOut --> preRouting.in[1]; 
        for i=0..numOfTransportProt-1 do 
            localDeliver.transportOut[i] --> transportOut[i]; 
        endfor; 

        // Multicast output
        multicast.localOut --> localDeliver.in[1]; 
        multicast.fragmentationOut --> fragmentation.multicastIn; 
        multicast.tunnelOut --> tunneling.multicastIn; 
                                                       
        // ICMP output, ping input
        icmp.sendOut --> send.in[0]; 
        icmp.pingOut --> ICMPOut; 
        icmp.errorOut --> errorHandling.in; 
        ICMPIn --> icmp.pingIn; 
                                
        // IP send output, input
        send.routingOut --> routing.in[1]; 
        for i=0..numOfTransportProt-1 do 
            transportIn[i] --> send.in[i+2]; 
        endfor; 

        // IP Tunnel output
        tunneling.sendOut --> send.in[1]; 
                                          
        // Output ports
        for i=0..numOfPorts-1 do 
            fragmentation.outputOut[i] --> output[i].in; 
            output[i].queueOut --> queueOut[i]; 
        endfor; 
endmodule 

