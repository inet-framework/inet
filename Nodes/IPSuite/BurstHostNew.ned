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
    "NetworkLayersNew",
    "TrafficGen",
    "LinkLayer";

//
// Definition of an IPNode with transport layers.
//
module BurstHostNew
    parameters:
        nodenr : numeric const,
        burstPackets : numeric const,
        packetSize : numeric const,
        nodename : string,
        numPorts : numeric const,
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
            display: "p=94,49;i=app;b=35,26";
        // control module
        networkLayer: NetworkLayerNew;
            parameters:
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                physIn[numPorts],
                physOut[numPorts];
            display: "b=32,30;p=95,116;i=fork";
        linkLayer: LinkLayer[numPorts];
            parameters:
                NWIName = "PPPModule";
            display: "p=82,202;i=prot2;b=32,30";
    connections nocheck:
        // transport connections
        networkLayer.UDPOut --> burstGenerator.in;
        networkLayer.UDPIn <-- burstGenerator.out;

        // network interfaces
        for i=0..numPorts do
            linkLayer[i].physOut --> networkLayer.physIn[i];
            linkLayer[i].nwIn <-- networkLayer.physOut[i];
            out[i] <-- linkLayer[i].physOut;
            in[i] --> linkLayer[i].physIn;
        endfor;
endmodule

