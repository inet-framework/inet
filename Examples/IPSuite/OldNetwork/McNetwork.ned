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
    "UDPHost";



channel standardChannel
endchannel

module MCNetwork
    submodules:
        // hosts
        host1: UDPHost;
            parameters:
                // network parameters
                numOfPorts = 2,
                routingFile = "OldNetwork/mchost1.mrt",
                // UDP app parameters
                packetTime = 10 s,
                packetSize = 1000,
                //packetTime = input(0.5us),
                //packetSize = input(1000),
                nodenr = 1,
                nodename = "host 1";
            gatesizes:
                in[2],
                out[2];
            display: "p=52,43;b=40,24,rect";
        host2: UDPHost;
            parameters:
                // network parameters
                numOfPorts = 2,
                routingFile = "OldNetwork/mchost2.mrt",
                // UDP app parameters
                packetTime = 20 s,
                packetSize = 1000,
                //packetTime = input(0.5us),
                //packetSize = input(1000),
                nodenr = 2,
                nodename = "host 2";
            gatesizes:
                in[2],
                out[2];
            display: "p=142,48;b=40,24,rect";
        host3: UDPHost;
            parameters:
                // network parameters
                numOfPorts = 1,
                routingFile = "OldNetwork/mchost3.mrt",
                // UDP app parameters
                packetTime = 200 s,
                packetSize = 1000,
                //packetTime = input(0.5us),
                //packetSize = input(1000),
                nodenr = 3,
                nodename = "host 3";
            gatesizes:
                in[1],
                out[1];
            display: "p=54,185;b=40,24,rect";
        host4: UDPHost;
            parameters:
                // network parameters
                numOfPorts = 1,
                routingFile = "OldNetwork/mchost4.mrt",
                // UDP app parameters
                packetTime = 500 s,
                packetSize = 1000,
                //packetTime = input(0.5us),
                //packetSize = input(1000),
                nodenr = 4,
                nodename = "host 4";
            gatesizes:
                in[1],
                out[1];
            display: "p=237,40;b=40,24,rect";
        host5: UDPHost;
            parameters:
                // network parameters
                numOfPorts = 1,
                routingFile = "OldNetwork/mchost5.mrt",
                // UDP app parameters
                packetTime = 100 s,
                packetSize = 1000,
                //packetTime = input(0.5us),
                //packetSize = input(1000),
                nodenr = 5,
                nodename = "host 5";
            gatesizes:
                in[1],
                out[1];
            display: "p=225,181;b=40,24,rect";
        router1: Router;
            parameters:
                // network parameters
                numOfPorts = 4,
                routingFile = "OldNetwork/mcrouter1.mrt",
                nodename = "router 1";
            gatesizes:
                in[4],
                out[4];
            display: "p=52,104;b=40,24,rect";
        router2: Router;
            parameters:
                // network parameters
                numOfPorts = 3,
                routingFile = "OldNetwork/mcrouter2.mrt",
                nodename = "router 2";
            gatesizes:
                in[3],
                out[3];
            display: "p=212,117;b=40,24,rect";
    connections nocheck:
        host1.out[0] --> standardChannel --> router1.in[0];
        host1.in[0] <-- standardChannel <-- router1.out[0];
        host1.out[1] --> standardChannel --> host2.in[1];
        host1.in[1] <-- standardChannel <-- host2.out[1];

        host2.out[0] --> standardChannel --> router1.in[1];
        host2.in[0] <-- standardChannel <-- router1.out[1];

        host3.out[0] --> standardChannel --> router1.in[2];
        host3.in[0] <-- standardChannel <-- router1.out[2];

        host4.out[0] --> standardChannel --> router2.in[0];
        host4.in[0] <-- standardChannel <-- router2.out[0];

        host5.out[0] --> standardChannel --> router2.in[1];
        host5.in[0] <-- standardChannel <-- router2.out[1];

        router1.out[3] --> router2.in[2];
        router1.in[3] <-- router2.out[2];
    display: "p=10,10;b=257,209,rect";
endmodule

network mcnw1 : MCNetwork
endnetwork

