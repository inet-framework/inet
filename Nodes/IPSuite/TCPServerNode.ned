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
    "LinkLayer",
    "NetworkLayer",
    "TCPUpperLayers";


//
// Definition of an IPNode with transport layers.
//
module TCPServerNode
    parameters:
        // TCP information
        local_addr : numeric,
        // Network information
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=60,60";
        // transport and application layers
        tcpApp: TCPUpperLayers;
            parameters:
                tcpApplType = "TCPServer",
                local_addr = local_addr,
                server_addr = 0;
            display: "p=84,61;b=40,24,rect";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
        netIf : LinkLayer[numOfPorts];
            parameters:
                NWIName = "PPPInterface";
            display: "p=80,220,row;i=iface";
    connections nocheck:
        // transport connections
        networkLayer.TCPOut --> tcpApp.in;
        networkLayer.TCPIn <-- tcpApp.out;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> netIf[i].physIn;
            out[i] <-- netIf[i].physOut;
            netIf[i].inputQueueOut --> networkLayer.physIn[i];
            netIf[i].outputQueueIn <-- networkLayer.physOut[i];
        endfor;
endmodule

