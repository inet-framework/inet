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
    "IPPreRouting",
    "IPRouting",
    "IPLocalDeliver",
    "IPMulticast",
    "IPSend",
    "IPFragmentation",
    "IPTunneling",
    "IPOutput",
    "IPInputQueue";



//
// Simulates the \IP protocol of a host or router. This module
// communicates with higher and upper layers in the same way
// as the IP module.
//
// author: Jochen Reber date: 2.5.00
// modifications Andras Varga
//
module IPProcessing
    parameters:
        timeToLive : numeric,
        multicastTimeToLive : numeric,
        IPForward : bool,
        fragmentTimeout : numeric const,
        protocolMapping: string;
    gates:
        in: transportIn[];
        out: transportOut[];
        in: queueIn[];
        out: queueOut[];
    submodules:
        preRouting: IPPreRouting;
            gatesizes:
                in[2];
            display: "b=32,30;p=95,177;i=prot3";
        routing: IPRouting;
            gatesizes:
                in[2];
            display: "b=34,34;p=179,177;i=bwgen";
        localDeliver: IPLocalDeliver;
            parameters:
                fragmentTimeout = fragmentTimeout,
                protocolMapping = protocolMapping;
            gatesizes:
                in[2],
                transportOut[sizeof(transportOut)];
            display: "b=32,30;p=95,78;i=fork";
        multicast: IPMulticast;
            display: "b=17,17;p=305,175;i=bwgen_s";
        send: IPSend;
            parameters:
                timeToLive = timeToLive,
                multicastTimeToLive = multicastTimeToLive;
            gatesizes:
                in[sizeof(transportIn)+2];
            display: "b=32,30;p=250,78;i=prot1";
        fragmentation: IPFragmentation;
            gatesizes:
                outputOut[sizeof(queueOut)];
            display: "b=32,30;p=231,249;i=bwxconn";
        tunneling: IPTunneling;
            display: "b=16,15;p=370,152;i=bwxcon_s";
        output: IPOutput[sizeof(queueOut)];
            display: "b=32,30;p=110,304,row;i=prot2";
        inputQueue: IPInputQueue;
            gatesizes:
                fromNW[sizeof(queueIn)];
            display: "p=50,304;i=queue";
    connections nocheck:

        // L2 input to IPInputQueue
        for i=0..sizeof(queueIn)-1 do
            queueIn[i] --> inputQueue.fromNW[i];
        endfor;

        // IPPreRouting Input
        inputQueue.toIP --> preRouting.in[0];

        // IPPreRouting Output
        preRouting.routingOut --> routing.in[0];

        // IPRouting output
        routing.localOut --> localDeliver.in[0];
        routing.multicastOut --> multicast.routingIn;
        routing.fragmentationOut --> fragmentation.routingIn;

        // IPLocalDeliver output
        localDeliver.preRoutingOut --> preRouting.in[1];
        for i=0..sizeof(transportOut)-1 do
            localDeliver.transportOut[i] --> transportOut[i];
        endfor;

        // Multicast output
        multicast.localOut --> localDeliver.in[1];
        multicast.fragmentationOut --> fragmentation.multicastIn;
        multicast.tunnelOut --> tunneling.multicastIn;

        // IP send output, input
        send.routingOut --> routing.in[1];
        for i=0..sizeof(transportIn)-1 do
            transportIn[i] --> send.in[i+2];
        endfor;

        // IP Tunnel output
        tunneling.sendOut --> send.in[1];

        // Output ports
        for i=0..sizeof(queueOut)-1 do
            fragmentation.outputOut[i] --> output[i].in;
            output[i].queueOut --> queueOut[i];
        endfor;
endmodule

