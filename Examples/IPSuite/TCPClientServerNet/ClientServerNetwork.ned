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
    "StandardHost2";


module ClientServerNetwork
    submodules:
        client1: StandardHost2;
            parameters:
                // TCP parameters
                local_addr = (10 << 24) + 1,
                server_addr = (10 << 24) + 4,
                // network parameters
                routingFile = "client1.irt";
            gatesizes:
                in[2],
                out[2];
            display: "p=66,50;i=comp";
        // clients
        client2: StandardHost2;
            parameters:
                // TCP parameters
                local_addr = (10 << 24) + 2,
                server_addr = (10 << 24) + 4,
                // network parameters
                routingFile = "client2.irt";
            gatesizes:
                in[1],
                out[1];
            display: "p=201,45;i=comp";
        client3: StandardHost2;
            parameters:
                // TCP parameters
                local_addr = (10 << 24) + 3,
                server_addr = (10 << 24) + 4,
                // network parameters
                routingFile = "client3.irt";
            gatesizes:
                in[1],
                out[1];
            display: "p=208,164;i=comp";
        server4: StandardHost2;
            parameters:
                // TCP parameters
                local_addr = (10 << 24) + 4,
                // network parameters
                routingFile = "server4.irt";
            gatesizes:
                in[2],
                out[2];
            display: "p=63,161;i=pc";
        router: Router;
            parameters:
                // network parameters
                routingFile = "router.irt";
            gatesizes:
                in[4],
                out[4];
            display: "p=138,102;i=ipc";
    connections nocheck:

        client1.out[0] --> datarate 1000000 --> router.in[0];
        client1.in[0] <-- datarate 1000000 <-- router.out[0];

        client2.out[0] --> datarate 1000000 --> router.in[1];
        client2.in[0] <-- datarate 1000000 <-- router.out[1];

        client3.out[0] --> datarate 1000000 --> router.in[2];
        client3.in[0] <-- datarate 1000000 <-- router.out[2];

        server4.out[0] --> datarate 1000000 --> router.in[3];
        server4.in[0] <-- datarate 1000000 <-- router.out[3];

        client1.out[1] --> datarate 1000000 --> server4.in[1];
        client1.in[1] <-- datarate 1000000 <-- server4.out[1];

endmodule

network tcpclservnet : ClientServerNetwork
endnetwork

