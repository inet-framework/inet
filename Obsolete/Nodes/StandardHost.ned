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
    "UDPUpperLayers",
    "TCPUpperLayers",
    "PingApp";



//
// Definition of an IPNode with transport layers.
//
module StandardHost
    parameters:
        // TCP information
        tcpApplType : string,
        local_addr : numeric,
        server_addr : numeric,

        // Network information
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=block/blackboard;p=60,60";
        // transport and application layers
        tcpApp: TCPUpperLayers;
            parameters:
                tcpApplType = tcpApplType,
                local_addr = local_addr,
                server_addr = server_addr;
            display: "p=60,50;b=40,24,rect";
        udpApp: UDPUpperLayers;
            parameters:
                udpClient1Name = "UDPClientApp",
                udpServer1Name = "UDPServerApp";
            display: "p=120,50;b=40,24,rect";
        pingApp: PingApp;
            display: "p=180,50;b=40,24,rect";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "p=120,110;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=60,180,row;i=block/ifcard";
    connections nocheck:
        // transport connections
        networkLayer.TCPOut --> tcpApp.in;
        networkLayer.TCPIn <-- tcpApp.out;

        networkLayer.UDPOut --> udpApp.from_ip;
        networkLayer.UDPIn <-- udpApp.to_ip;

        networkLayer.pingOut --> pingApp.in;
        networkLayer.pingIn <-- pingApp.out;

        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;
endmodule

