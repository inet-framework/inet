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
    datarate 10*10^6;
endchannel

channel dialup
    delay normal(0.004,0.0018);
    datarate 64*10^3;
endchannel

channel fiberrouter
    delay normal(0.000001,0.0000001);
    datarate 512*10^6;
endchannel

module MulticastNetwork
    submodules:
        host1: StandardHost;
            parameters:
                IPForward = false,
                routingFile = "mchost1.mrt";
            gatesizes:
                in[1],
                out[1];
            display: "p=125,52;i=device/pc2";
        host2: StandardHost;
            parameters:
                IPForward = true,
                routingFile = "mchost2.mrt";
            gatesizes:
                in[2],
                out[2];
            display: "p=60,244;i=device/pc2";
        host3: StandardHost;
            parameters:
                IPForward = true,
                routingFile = "mchost3.mrt";
            gatesizes:
                in[2],
                out[2];
            display: "p=196,244;i=device/pc2";
        host4: StandardHost;
            parameters:
                IPForward = false,
                routingFile = "mchost4.mrt";
            gatesizes:
                in[1],
                out[1];
            display: "p=406,305;i=device/pc2";
        host5: StandardHost;
            parameters:
                IPForward = false,
                routingFile = "mchost5.mrt";
            gatesizes:
                in[1],
                out[1];
            display: "p=492,59;i=device/pc2";
        host6: StandardHost;
            parameters:
                IPForward = false,
                routingFile = "mchost6.mrt";
            gatesizes:
                in[1],
                out[1];
            display: "p=492,204;i=device/pc2";
        router1: Router;
            parameters:
                routingFile = "mcrouter1.mrt";
            gatesizes:
                in[4],
                out[4];
            display: "p=124,135;i=abstract/router";
        router2: Router;
            parameters:
                routingFile = "mcrouter2.mrt";
            gatesizes:
                in[3],
                out[3];
            display: "p=292,135;i=abstract/router";
        router3: Router;
            parameters:
                routingFile = "mcrouter3.mrt";
            gatesizes:
                in[2],
                out[2];
            display: "p=292,238;i=abstract/router";
        router4: Router;
            parameters:
                routingFile = "mcrouter4.mrt";
            gatesizes:
                in[3],
                out[3];
            display: "p=408,135;i=abstract/router";
    connections:
        // * Domain 1 * (172.0.0.x)
        host1.out[0] --> ethernet --> router1.in[0];
        host1.in[0] <-- ethernet <-- router1.out[0];

        host2.out[0] --> ethernet --> router1.in[1];
        host2.in[0] <-- ethernet <-- router1.out[1];
        host2.out[1] --> dialup --> host3.in[1];
        host2.in[1] <-- dialup <-- host3.out[1];

        host3.out[0] --> ethernet --> router1.in[2];
        host3.in[0] <-- ethernet <-- router1.out[2];

        // * Domain 2 * (172.0.1.x)
        host4.out[0] --> ethernet --> router3.in[0];
        host4.in[0] <-- ethernet <-- router3.out[0];

        // * Domain 3 * (172.0.2.x)
        host5.out[0] --> dialup --> router4.in[0];
        host5.in[0] <-- dialup <-- router4.out[0];

        host6.out[0] --> dialup --> router4.in[1];
        host6.in[0] <-- dialup <-- router4.out[1];

        // * Router Connections * (172.1.0.x)
        router2.out[0] --> fiberrouter --> router1.in[3];
        router2.in[0] <-- fiberrouter <-- router1.out[3];

        router2.out[1] --> fiberrouter --> router3.in[1];
        router2.in[1] <-- fiberrouter <-- router3.out[1];

        router2.out[2] --> fiberrouter --> router4.in[2];
        router2.in[2] <-- fiberrouter <-- router4.out[2];
endmodule

network multicastNetwork : MulticastNetwork
endnetwork

