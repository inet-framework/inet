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
    "PingApp",
    "TrafficGen";




//
// Definition of an IPNode with transport layers.
//
module BurstHost
    parameters:

        nodenr : numeric const,
        burstPackets : numeric const,
        packetSize : numeric const,
        nodename : string,

        // Network information
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        burstGenerator: TrafficGenerator;
            parameters:
                GeneratorType = "BurstGenerator",
                nodename = nodename,
                nodenr = nodenr,
                tcpProtocol = false,
                burstPackets = burstPackets,
                generationTime = 0,
                generationSize = packetSize;
        // control module
        networkLayers: NetworkLayers;
            parameters:
                // other Network parameters
                IPForward = false,
                NWIName = "PPPModule",
                numOfPorts = 1,
                routingFile = routingFile;
            gatesizes:
                physIn[1],
                physOut[1];
            display: "b=32,30;p=87,155;i=fork";
    connections nocheck:

                         // transport connections

        networkLayers.UDPOut --> burstGenerator.in;
        networkLayers.UDPIn <-- burstGenerator.out;

        // connections to network outside
        for i=0..0 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

