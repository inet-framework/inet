//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//



package inet.nodes.inet;

import inet.networklayer.arp.ARP;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.IGMP;
import inet.networklayer.ipv4.IP;


//
// Network layer of an \IP node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module NetworkLayer
{
    parameters:
        bool proxyARP = default(true);
        @display("i=block/fork");
    gates:
        input ifIn[];
        input tcpIn;
        input udpIn;
        input sctpIn;   //I.R.
        input rsvpIn;
        input ospfIn;
        input pingIn;
        output ifOut[];
        output TCPOut;
        output udpOut;
        output sctpOut; //I.R.
        output rsvpOut;
        output ospfOut;
        output pingOut;

    submodules:
        ip: IP {
            parameters:
                timeToLive = 32;
                multicastTimeToLive = 32;
                fragmentTimeout = 60s;
                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6";   //I.R.
                @display("p=85,95;q=queue");
            gates:
                transportIn[7]; //I.R.
                transportOut[7];
                queueIn[sizeof(ifIn)];
        }
        arp: ARP {
            parameters:
                proxyARP = proxyARP;
                @display("p=163,206;q=pendingQueue");
            gates:
                nicOut[sizeof(ifOut)];
        }
        icmp: ICMP {
            parameters:
                @display("p=160,63");
        }
        igmp: IGMP {
            parameters:
                @display("p=160,122");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=239,63");
        }
    connections allowunconnected:
        // transport Layer
        ip.transportOut[0] --> TCPOut;
        ip.transportIn[0] <-- tcpIn;

        ip.transportOut[1] --> udpOut;
        ip.transportIn[1] <-- udpIn;

        ip.transportOut[2] --> icmp.localIn;
        ip.transportIn[2] <-- icmp.sendOut;

        ip.transportOut[3] --> igmp.localIn;
        ip.transportIn[3] <-- igmp.sendOut;

        ip.transportOut[4] --> rsvpOut;
        ip.transportIn[4] <-- rsvpIn;

        ip.transportOut[5] --> ospfOut;
        ip.transportIn[5] <-- ospfIn;

        ip.transportOut[6] --> sctpOut;     //I.R.
        ip.transportIn[6] <-- sctpIn;

        icmp.pingOut --> pingOut;
        icmp.pingIn <-- pingIn;

        icmp.errorOut --> errorHandling.in;

        ip.queueOut --> arp.ipIn;

        // L2 interfaces to IP and from ARP
        for i=0..sizeof(ifOut)-1 {
            ifIn[i] --> { @display("m=s"); } --> ip.queueIn[i];
            ifOut[i] <-- { @display("m=s"); } <-- arp.nicOut[i];
        }
}

