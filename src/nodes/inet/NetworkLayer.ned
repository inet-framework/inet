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

import inet.networklayer.ipv4.IP;
import inet.networklayer.ipv4.IGMP;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.arp.ARP;
import inet.base.IHook;


//
// Network layer of an IP node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module NetworkLayer
{
    parameters:
        bool proxyARP = default(true);
        @display("i=block/fork");
    gates:
        input ifIn[] @labels(IPDatagram);
        input tcpIn @labels(TCPSegment,IPControlInfo/down);
        input udpIn @labels(UDPPacket,IPControlInfo/down);
        input sctpIn @labels(IPControlInfo/down,SCTPPacket);
        input rsvpIn @labels(IPControlInfo/down);
        input ospfIn @labels(IPControlInfo/down);
        input pingIn;
        output ifOut[];
        output tcpOut @labels(TCPSegment,IPControlInfo/up);
        output udpOut @labels(UDPPacket,IPControlInfo/up);
        output sctpOut @labels(IPControlInfo/up,SCTPPacket);
        output rsvpOut @labels(IPControlInfo/up);
        output ospfOut @labels(IPControlInfo/up);
        output pingOut;

    submodules:
        ip: IP {
            parameters:
                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6";
                @display("p=85,95;q=queue");
            gates:
                transportIn[7];
                transportOut[7];
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

        // a placeholder in the traffic coming from the network interfaces 
        inputHook[sizeof(ifIn)]: <default("Nop")> like IHook {
            @display("p=85,254");
        }
        // a placeholder if traffic going to the network interfaces
        outputHook[sizeof(ifOut)]: <default("Nop")> like IHook {
            @display("p=202,232");
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

        icmp.pingOut --> pingOut;
        icmp.pingIn <-- pingIn;

        icmp.errorOut --> errorHandling.in;

        ip.queueOut --> arp.ipIn;

        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> inputHook[i].in;
            inputHook[i].out --> ip.queueIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            arp.nicOut[i] --> outputHook[i].in;
            outputHook[i].out --> ifOut[i];
        }
}

