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
    "InputQueue";

//
// Network layers of the IP Node. Contains: IPProcessing, IP input queue,
// and Routing Table
//
// Interfaces to transport layer: TCP, UDP, echo/ping, error out
//
module NetworkLayerNew
    parameters:
        IPForward : bool,
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: physIn[];
        in: TCPIn;
        in: UDPIn;
        in: pingIn;
        out: physOut[];
        out: TCPOut;
        out: UDPOut;
        out: pingOut;

    submodules:
        ip: IPProcessing;
            parameters:
                IPForward = IPForward,
                numOfPorts = numOfPorts,
                numOfTransportProt = 2,
                timeToLive = 8,
                multicastTimeToLive = 6,
                fragmentTimeout = "5s";
            gatesizes:
                transportIn[2],
                transportOut[2],
                queueOut[numOfPorts];
            display: "b=32,30;p=123,79;i=cogwheel";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routingTableFileName = routingFile;
            display: "b=32,32;p=190,40,row;i=box3";
        inputQueue: InputQueue;
            gatesizes:
                fromNW[numOfPorts];
            display: "b=32,30;p=79,156;i=queue";
    connections nocheck:
        // transport Layer
        ip.transportOut[0] --> TCPOut;
        ip.transportIn[0] <-- TCPIn;

        ip.transportOut[1] --> UDPOut;
        ip.transportIn[1] <-- UDPIn;

        // ICMP interface
        ip.ICMPOut --> pingOut;
        ip.ICMPIn <-- pingIn;

        inputQueue.toIP --> ip.queueIn;
        for i=0..numOfPorts-1 do
            physOut[i] <-- ip.queueOut[i] display "m=s";
            physIn[i] --> inputQueue.fromNW[i] display "m=s";
        endfor;
endmodule

