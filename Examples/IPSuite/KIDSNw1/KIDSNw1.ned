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
    "KIDSRouter",
    "UDPHost";


channel ethernet
    delay normal(0.00015,0.00005);
    datarate 10*1000000;
endchannel

channel fiberrouterline
    delay normal(0.000001,0.0000001);
    datarate 512*1000000;
endchannel

module KIDSNw1
    submodules:
        // *** Hosts ***
        host: UDPHost[3];
            parameters:
                IPForward = false,
                numOfPorts = 1,
                udpRouteDestNo = 2,
                packetSize = 10000;
            parameters if index==0:
                routingFile = "host1.irt",
                nodenr = 1,
                packetTime =  0.002 s,
                nodename = "host 1";
            parameters if index==1:
                routingFile = "host2.irt",
                nodenr = 2,
                packetTime = 0.002 s,
                nodename = "host 2";
            parameters if index==2:
                routingFile = "host3.irt",
                nodenr = 3,
                packetTime = 0.002 s,
                nodename = "host 3";
            gatesizes:
                in[1],
                out[1];

            display: "p=102,209,row;i=pc";
        // *** Router ****
        // First Hop Router
        fhrouter: KIDSRouter[2];
            parameters if index==0:
                routingFile = "fhrouter1.irt",
                nodename = "First Hop Router 1",
                numOfPorts = 3;
            parameters if index==1:
                routingFile = "fhrouter5.irt",
                nodename = "First Hop Router 5",
                numOfPorts = 2;
            gatesizes:
                in[2],
                out[2];
            gatesizes if index==0:
                in[3],
                out[3];

            display: "p=102,153,row;i=ipc";
        // InteriorRouter
        irouter: KIDSRouter;
            parameters:
                routingFile = "irouter2.irt",
                nodename = "Interior Router 2",
                numOfPorts = 2;
            gatesizes:
                in[2],
                out[2];

            display: "p=116,116;i=ipc";
        // Border Gatgeway Router
        bgrouter: KIDSRouter[2];
            parameters:
                numOfPorts = 2;
            parameters if index==0:
                routingFile = "bgrouter3.irt",
                nodename = "Border Gateway Router 3";
            parameters if index==1:
                routingFile = "bgrouter4.irt",
                nodename = "Border Gateway Router 4";
            gatesizes:
                in[2],
                out[2];

            display: "p=135,64,row;i=router";
    connections nocheck:
        // * host connections *
        host[0].out[0] --> ethernet --> fhrouter[0].in[0];
        host[0].in[0] <-- ethernet <-- fhrouter[0].out[0];
        host[1].out[0] --> ethernet --> fhrouter[0].in[1];
        host[1].in[0] <-- ethernet <-- fhrouter[0].out[1];
        host[2].out[0] --> ethernet --> fhrouter[1].in[0];
        host[2].in[0] <-- ethernet <-- fhrouter[1].out[0];

        // * Router Connections *
        fhrouter[0].out[2] --> fiberrouterline --> irouter.in[0];
        fhrouter[0].in[2] <-- fiberrouterline <-- irouter.out[0];

        irouter.out[1] --> fiberrouterline --> bgrouter[0].in[0];
        irouter.in[1] <-- fiberrouterline <-- bgrouter[0].out[0];

        bgrouter[0].out[1] --> fiberrouterline --> bgrouter[1].in[1];
        bgrouter[0].in[1] <-- fiberrouterline <-- bgrouter[1].out[1];

        bgrouter[1].out[0] --> fiberrouterline --> fhrouter[1].in[1];
        bgrouter[1].in[0] <-- fiberrouterline <-- fhrouter[1].out[1];

endmodule

network kidsnw1 : KIDSNw1
endnetwork

