//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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
    "NetworkLayer",
    "UDPProcessing",
    "SocketApp",
    "socketlayer";


// FIXME socketLayer should get a local_port parameter, UDPProcessing needs that (?!?!)
module UDPSocketHost
    parameters:
        client_name : string,
        routingFile : string;
    gates:
        in: in[];
        out: out[];

    submodules:
        applClient: client_name like SocketApp;
            display: "p=80,40;i=proc1";
        socketLayer: SocketLayer;
            display: "p=80,100;i=block/wheelbarrow";
        udp: UDPProcessing;
            display: "p=80,160;i=block/transport";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "p=80,220;i=block/fork";
        ppp : PPPInterface[sizeof(out)];
            display: "p=80,280,row;q=queue;i=block/ifcard";
    connections nocheck:
        // transport connections
        applClient.out --> socketLayer.from_appl[0];
        applClient.in <-- socketLayer.to_appl[0];
        socketLayer.to_udp --> udp.from_application;
        socketLayer.from_udp <-- udp.to_application;
        networkLayer.UDPOut --> udp.from_ip;
        networkLayer.UDPIn <-- udp.to_ip;

        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;
endmodule

