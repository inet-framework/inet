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
    "StandardHost2",
    "FlatNetworkConfigurator";


module ClientServerNetwork
    submodules:
        client1: StandardHost2;
            display: "p=120,60;i=comp";
        client2: StandardHost2;
            display: "p=260,60;i=comp";
        client3: StandardHost2;
            display: "p=260,160;i=comp";
        server: StandardHost2;
            display: "p=120,160;i=pc";
        router: Router;
            display: "p=190,102;i=ipc";
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypesToProcess = "Router StandardHost2",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=50,70;i=cogwheel";
    connections:
        client1.out++ --> datarate 1000000 --> router.in++;
        client1.in++ <-- datarate 1000000 <-- router.out++;

        client2.out++ --> datarate 1000000 --> router.in++;
        client2.in++ <-- datarate 1000000 <-- router.out++;

        client3.out++ --> datarate 1000000 --> router.in++;
        client3.in++ <-- datarate 1000000 <-- router.out++;

        server.out++ --> datarate 1000000 --> router.in++;
        server.in++ <-- datarate 1000000 <-- router.out++;

        client1.out++ --> datarate 1000000 --> server.in++;
        client1.in++ <-- datarate 1000000 <-- server.out++;
endmodule

network tcpclservnet : ClientServerNetwork
endnetwork

