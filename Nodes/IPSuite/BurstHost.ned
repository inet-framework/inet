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
    "Blackboard",
    "NetworkLayer",
    "TrafficGen";




//
// Definition of an IPNode with transport layers.
//
module BurstHost
    parameters:
        burstPackets : numeric const,
        packetSize : numeric const,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=60,60";
        burstGenerator: TrafficGenerator;
            parameters:
                GeneratorType = "BurstGenerator",
                tcpProtocol = false,
                burstPackets = burstPackets,
                generationTime = 0,
                generationSize = packetSize;
            display: "b=32,30;p=80,60";
        networkLayer: NetworkLayer;
            parameters:
                // other Network parameters
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                physIn[1],
                physOut[1];
            display: "b=32,30;p=80,120;i=fork";
        ppp : PPPInterface[1];
            display: "p=80,180,row;i=iface";
    connections nocheck:
        // transport connections
        networkLayer.UDPOut --> burstGenerator.in;
        networkLayer.UDPIn <-- burstGenerator.out;

        // connections to network outside
        for i=0..0 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;

endmodule

