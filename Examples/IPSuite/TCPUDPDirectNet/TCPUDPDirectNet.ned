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



module TCPUDPDirectNet
    submodules:
        // server
        server1: StandardHost2;
            parameters:
                // TCP parameters
                tcpApplType = "TCPServer",
                local_addr = (10 << 24) + 1,
                server_addr = (10 << 24) + 0,
                // network parameters
                routingFile = "server1.irt";
            gatesizes:
                in[3],
                out[3];
            display: "p=40,80;i=pc";
        // clients
        client2: StandardHost2;
            parameters:
                // TCP parameters
                tcpApplType = "TCPClient",
                local_addr = (10 << 24) + 2,
                server_addr = (10 << 24) + 1,
                // network parameters
                routingFile = "client2.irt";
            gatesizes:
                in[3],
                out[3];
            display: "p=200,80;i=comp";
        client3: StandardHost2;
            parameters:
                // TCP parameters
                tcpApplType = "TCPClient",
                local_addr = (10 << 24) + 3,
                server_addr = (10 << 24) + 1,
                // network parameters
                routingFile = "client3.irt";
            gatesizes:
                in[3],
                out[3];
            display: "p=40,200;i=bwcomp";
        client4: StandardHost2;
            parameters:
                // TCP parameters
                tcpApplType = "TCPClient",
                local_addr = (10 << 24) + 4,
                server_addr = (10 << 24) + 1,
                // network parameters
                routingFile = "client4.irt";
            gatesizes:
                in[3],
                out[3];
            display: "p=200,200;i=comp";
    connections:
        server1.out[0] --> datarate 1000000 --> client2.in[0];
        server1.out[1] --> datarate 1000000 --> client3.in[0];
        server1.out[2] --> datarate 1000000 --> client4.in[0];

        client2.out[0] --> datarate 1000000 --> server1.in[0];
        client2.out[1] --> datarate 1000000 --> client3.in[1];
        client2.out[2] --> datarate 1000000 --> client4.in[1];

        client3.out[0] --> datarate 1000000 --> server1.in[1];
        client3.out[1] --> datarate 1000000 --> client2.in[1];
        client3.out[2] --> datarate 1000000 --> client4.in[2];

        client4.out[0] --> datarate 1000000 --> server1.in[2];
        client4.out[1] --> datarate 1000000 --> client2.in[2];
        client4.out[2] --> datarate 1000000 --> client3.in[2];
endmodule

network directnw : TCPUDPDirectNet
endnetwork

