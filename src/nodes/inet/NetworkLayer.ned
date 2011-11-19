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

import inet.networklayer.ipv4.IPv4;
import inet.networklayer.ipv4.IGMP;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.arp.ARP;


//
// Network layer of an IPv4 node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module NetworkLayer
{
    parameters:
        @display("i=block/fork");
        bool proxyARP = default(true);
    gates:
        input ifIn[] @labels(IPv4Datagram);
        input tcpIn @labels(TCPSegment,IPv4ControlInfo/down);
        input udpIn @labels(UDPPacket,IPv4ControlInfo/down);
        input sctpIn @labels(IPv4ControlInfo/down,SCTPPacket);
        input rsvpIn @labels(IPv4ControlInfo/down);
        input ospfIn @labels(IPv4ControlInfo/down);
        input pingIn;
        input manetIn;
        output ifOut[];
        output tcpOut @labels(TCPSegment,IPv4ControlInfo/up);
        output udpOut @labels(UDPPacket,IPv4ControlInfo/up);
        output sctpOut @labels(IPv4ControlInfo/up,SCTPPacket);
        output rsvpOut @labels(IPv4ControlInfo/up);
        output ospfOut @labels(IPv4ControlInfo/up);
        output pingOut;
        output manetOut;

    submodules:
        ip: IPv4 {
            parameters:
                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6,48:7,138:7"; // see: http://www.iana.org/assignments/protocol-numbers
                @display("p=85,95;q=queue");
            gates:
                transportIn[8];
                transportOut[8];
                queueIn[sizeof(ifIn)];
        }
        arp: ARP {
            parameters:
                proxyARP = proxyARP;
                @display("p=202,142;q=pendingQueue");
            gates:
                nicOut[sizeof(ifOut)];
        }
        icmp: ICMP {
            parameters:
                @display("p=160,63");
        }
        igmp: IGMP {
            parameters:
                @display("p=85,30");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=239,63");
        }

    connections allowunconnected:
        // transport Layer
        ip.transportOut[0] --> tcpOut;
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

        ip.transportOut[6] --> sctpOut;
        ip.transportIn[6] <-- sctpIn;

        ip.transportOut[7] --> manetOut;
        ip.transportIn[7] <-- manetIn;

        icmp.pingOut --> pingOut;
        icmp.pingIn <-- pingIn;

        icmp.errorOut --> errorHandling.in;

        ip.queueOut --> arp.ipIn;

        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> ip.queueIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            arp.nicOut[i] --> ifOut[i];
        }
}

