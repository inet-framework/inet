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
    "Router",
    "StandardHost";


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
        host: StandardHost[3];
            parameters:
                IPForward = false;
            parameters if index==0:
                routingFile = "host1.irt";
            parameters if index==1:
                routingFile = "host2.irt";
            parameters if index==2:
                routingFile = "host3.irt";
            gatesizes:
                in[1],
                out[1];
            display: "p=101,307,row;i=device/pc2";

        // First Hop Router
        fhrouter: Router[2];
            parameters if index==0:
                routingFile = "fhrouter1.irt";
            parameters if index==1:
                routingFile = "fhrouter5.irt";
            gatesizes:
                in[2],
                out[2];
            gatesizes if index==0:
                in[3],
                out[3];
            display: "p=119,222,row,130;i=device/router";

        // InteriorRouter
        irouter: Router;
            parameters:
                routingFile = "irouter2.irt";
            gatesizes:
                in[2],
                out[2];
            display: "p=115,144;i=device/router";
        // Border Gatgeway Router
        bgrouter: Router[2];
            parameters:
            parameters if index==0:
                routingFile = "bgrouter3.irt";
            parameters if index==1:
                routingFile = "bgrouter4.irt";
            gatesizes:
                in[2],
                out[2];
            display: "p=135,64,row;i=abstract/router";
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

