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
    "IPProcessing",
    "ICMP",
    "IGMP",
    "RoutingTable",
    "NetworkInterface";


//
// Network layers of an IP node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
// FIXME add ICMP errors
//
module NetworkLayer
    parameters:
        IPForward : bool,
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: physIn[]; // FIXME to be renamed
        in: TCPIn;
        in: UDPIn;
        in: RSVPIn;
        in: pingIn;
        out: physOut[]; // FIXME to be renamed
        out: TCPOut;
        out: UDPOut;
        out: RSVPOut;
        out: pingOut;

    submodules:
        ip: IPProcessing;
            parameters:
                IPForward = IPForward,
                numOfPorts = numOfPorts,
                timeToLive = 8,
                multicastTimeToLive = 6,
                fragmentTimeout = 60,
                protocolMapping = "6:0,17:1,1:2,2:3,46:4";
            gatesizes:
                transportIn[5],
                transportOut[5],
                queueIn[numOfPorts],
                queueOut[numOfPorts];
            display: "b=32,30;p=102,55;i=cogwheel";
        icmp: ICMP;
            display: "b=32,30;p=259,78;i=cogwheel";
        igmp: IGMP;
            display: "b=32,30;p=259,135;i=cogwheel";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routingTableFileName = routingFile;
            display: "b=32,32;p=185,53,row;i=box3";
        errorHandling: ErrorHandling;
            display: "b=16,16;p=330,78;i=box1_s";

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

        // L2 interfaces to IP
        for i=0..numOfPorts-1 do
            physIn[i] --> ip.queueIn[i] display "m=s";
            physOut[i] <-- ip.queueOut[i] display "m=s";
        endfor;
endmodule

