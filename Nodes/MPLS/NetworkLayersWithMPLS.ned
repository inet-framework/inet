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
    "RoutingTable",
    "InputQueue",
    "OutputQueue",
    "NetworkInterface";


//
// A version of NetworkLayers, with additional gates for OSPF and RSVP-TE
//
module NetworkLayersWithMPLS
    parameters:
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

    submodules:
        ip: IPProcessing;
            parameters:
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
            display: "b=32,30;p=130,127;i=queue";
        inputQueue: InputQueue;
            gatesizes:
                fromNW[numOfPorts];
            display: "p=39,53;i=queue";
        networkInterface: NWIName[numOfPorts] like NetworkInterface;
            display: "p=55,181;i=bwxconn";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routingTableFileName = routingFile;
            display: "p=238,64;i=box3";
    connections nocheck:
        // transport Layer
        ip.transportOut[0] --> TCPOut;
        ip.transportIn[0] <-- TCPIn;

        ip.transportOut[1] --> UDPOut;
        ip.transportIn[1] <-- UDPIn;

        ip.transportOut[2] --> OSPFOut;
        ip.transportIn[2] <-- OSPFIn;

        ip.transportOut[3] --> RSVPOut;
        ip.transportIn[3] <-- RSVPIn;

        // ICMP interface
        ip.ICMPOut --> pingOut;
        ip.ICMPIn <-- pingIn;

        // L2 IP Queues to IP
        inputQueue.toIP --> ip.queueIn;
        for i=0..numOfPorts-1 do
            ip.queueOut[i] --> outputQueue[i].fromIP;
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
endmodule

