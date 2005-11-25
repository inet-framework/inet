//
// Copyright (C) 2004 Andras Varga
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
    "IP",
    "ARP",
    "ICMP",
    "IGMP",
    "ErrorHandling";

//
// Network layer of an \IP node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module NetworkLayer
    parameters:
        proxyARP: bool;
    gates:
        in: ifIn[];
        in: TCPIn;
        in: UDPIn;
        in: RSVPIn;
        in: OSPFIn;
        in: pingIn;
        out: ifOut[];
        out: TCPOut;
        out: UDPOut;
        out: RSVPOut;
        out: OSPFOut;
        out: pingOut;

    submodules:
        ip: IP;
            parameters:
                timeToLive = 32,
                multicastTimeToLive = 32,
                fragmentTimeout = 60,
                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5";
            gatesizes:
                transportIn[6],
                transportOut[6],
                queueIn[sizeof(ifIn)];
            display: "p=85,95;i=block/routing;q=queue";
        arp: ARP;
            parameters:
                proxyARP = proxyARP;
            gatesizes:
                nicOut[sizeof(ifOut)];
            display: "p=163,206;i=block/layer;q=pendingQueue";
        icmp: ICMP;
            display: "p=160,63;i=block/control_s";
        igmp: IGMP;
            display: "p=160,122;i=block/cogwheel_s";
        errorHandling: ErrorHandling;
            display: "p=239,63;i=block/process_s";
    connections nocheck:
        // transport Layer
        ip.transportOut[0] --> TCPOut;
        ip.transportIn[0] <-- TCPIn;

        ip.transportOut[1] --> UDPOut;
        ip.transportIn[1] <-- UDPIn;

        ip.transportOut[2] --> icmp.localIn;
        ip.transportIn[2] <-- icmp.sendOut;

        ip.transportOut[3] --> igmp.localIn;
        ip.transportIn[3] <-- igmp.sendOut;

        ip.transportOut[4] --> RSVPOut;
        ip.transportIn[4] <-- RSVPIn;

        ip.transportOut[5] --> OSPFOut;
        ip.transportIn[5] <-- OSPFIn;

        icmp.pingOut --> pingOut;
        icmp.pingIn <-- pingIn;

        icmp.errorOut --> errorHandling.in;

        ip.queueOut --> arp.ipIn;

        // L2 interfaces to IP and from ARP
        for i=0..sizeof(ifOut)-1 do
            ifIn[i] --> ip.queueIn[i] display "m=s";
            ifOut[i] <-- arp.nicOut[i] display "m=s";
        endfor;
endmodule

