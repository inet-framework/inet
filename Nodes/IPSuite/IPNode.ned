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
    "NetworkLayers",
    "UDPUpperLayers",
    "tcp4ip",
    "PingApp",
    "TrafficGen";


//
// Definition of an IPNode with transport layers.
//
module IPNode
    parameters:
        // TCP information
        tcpApplType : string,
        local_addr : numeric,
        server_addr : numeric,

        // other application information
        packetSize : numeric const,  // (bit)
        packetTime : numeric const,
        nodename : string,
        nodenr : numeric const,
        udpRouteDestNo : numeric const,

        // Network information
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        // transport and application layers
//        tcpApp: TrafficGenerator;
//            parameters:
//              GeneratorType = "ConstTrafficGen",
//                nodename = nodename,
//              nodenr = nodenr,
//                tcpProtocol = true,
//              burstPackets = 0,
//                generationTime = packetTime,
//                generationSize = packetSize;
//            display: "p=36,93;b=40,24,rect";
        tcpApp: TCPUpperLayers;
            parameters:
                tcpApplType = tcpApplType,
                local_addr = local_addr,
                server_addr = server_addr;
          display: "p=36,93;b=40,24,rect";
        udpApp: UDPUpperLayers;
            parameters:
                nodename = nodename,
                udpCl1Name = "UDPClient",
                udpServer1Name = "UDPServer",
                generationTime = packetTime,
                generationSize = packetSize,
                routeDestNo = udpRouteDestNo;
            display: "p=97,92;b=40,24,rect";
        pingApp: PingApp;
            parameters:
                nodename = nodename,
                generationTime = packetTime;
            display: "p=152,89;b=40,24,rect";
        networkLayers: NetworkLayers;
            parameters:
                IPForward = true,
                NWIName = "PPPModule",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
    connections nocheck:
        // transport connections
        networkLayers.TCPOut --> tcpApp.in;
        networkLayers.TCPIn <-- tcpApp.out;

        networkLayers.UDPOut --> udpApp.from_ip;
        networkLayers.UDPIn <-- udpApp.to_ip;

        networkLayers.pingOut --> pingApp.in;
        networkLayers.pingIn <-- pingApp.out;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

