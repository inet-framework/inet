//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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





//
// Simulates the \IP protocol of a host or router. This module
// communicates with higher and upper layers in the same way
// as the IP module.
//
// author: Jochen Reber date: 2.5.00
// modifications Andras Varga
//
module IPProcessing
{
    parameters:
        volatile double timeToLive;
        volatile double multicastTimeToLive;
        bool IPForward;
        double fragmentTimeout;
        string protocolMapping;
    gates:
        input transportIn[];
        output transportOut[];
        input queueIn[];
        output queueOut;
    submodules:
        preRouting: IPPreRouting {
            parameters:
                @display("p=95,177");
            gates:
                in[2];
        };
        routing: IPRouting {
            parameters:
                @display("p=179,177");
            gates:
                in[2];
        };
        localDeliver: IPLocalDeliver {
            parameters:
                fragmentTimeout = fragmentTimeout;
                protocolMapping = protocolMapping;
                @display("p=95,78");
            gates:
                in[2];
                transportOut[sizeof(transportOut)];
        };
        multicast: IPMulticast {
            parameters:
                @display("p=305,175");
        };
        send: IPSend {
            parameters:
                timeToLive = timeToLive;
                multicastTimeToLive = multicastTimeToLive;
                @display("p=250,78");
            gates:
                in[sizeof(transportIn)+2];
        };
        fragmentation: IPFragmentation {
            parameters:
                @display("p=231,249");
        };
        tunneling: IPTunneling {
            parameters:
                @display("p=370,152");
        };
        output: IPOutput {
            parameters:
                @display("p=333,322");
        };
        inputQueue: IPInputQueue {
            parameters:
                @display("p=98,322");
            gates:
                fromNW[sizeof(queueIn)];
        };
    connections allowunconnected:

        // L2 input to IPInputQueue
        for i=0..sizeof(queueIn)-1 {
            queueIn[i] --> inputQueue.fromNW[i];
        };

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
        for i=0..sizeof(transportOut)-1 {
            localDeliver.transportOut[i] --> transportOut[i];
        };

        // Multicast output
        multicast.localOut --> localDeliver.in[1];
        multicast.fragmentationOut --> fragmentation.multicastIn;
        multicast.tunnelOut --> tunneling.multicastIn;

        // IP send output, input
        send.routingOut --> routing.in[1];
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> send.in[i+2];
        };

        // IP Tunnel output
        tunneling.sendOut --> send.in[1];

        // Output ports
        fragmentation.outputOut --> output.in;
        output.queueOut --> queueOut;

        // shortcut to send incoming \ARP packets towards ARP
        inputQueue.arpOut --> output.arpIn;
}

