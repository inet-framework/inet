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
                    
                    
 
module TCPUDPDirectNet 
    submodules: 
        // server
        server1: StandardHost; 
            parameters: 
                // TCP parameters
                tcpApplType = "TCPServer", 
                local_addr = (10 << 24) + 1, 
                server_addr = (10 << 24) + 0, 
                // UDP parameters
                packetTime = 0.2 s, 
                packetSize = 1000, 
                nodename = "server 1", 
                nodenr = 1, 
                udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 3, 
                routingFile = "server1.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=37,79;i=pc";
        // clients
        client2: StandardHost; 
            parameters: 
                // TCP parameters
                tcpApplType = "TCPClient", 
                local_addr = (10 << 24) + 2, 
                server_addr = (10 << 24) + 1, 
                // UDP parameters
                packetTime = 0.5 s, 
                packetSize = 1000, 
                nodename = "client 2", 
                nodenr = 2, 
                udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 3, 
                routingFile = "client2.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=202,83;i=comp";
        client3: StandardHost; 
            parameters: 
                // TCP parameters
                tcpApplType = "TCPClient", 
                local_addr = (10 << 24) + 3, 
                server_addr = (10 << 24) + 1, 
                // UDP parameters
                packetTime = 0.5 s, 
                packetSize = 200, 
                nodename = "client 3", 
                nodenr = 3, 
                udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 3, 
                routingFile = "client3.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=42,208;i=bwcomp";
        client4: StandardHost; 
            parameters: 
                // TCP parameters
                tcpApplType = "TCPClient", 
                local_addr = (10 << 24) + 4, 
                server_addr = (10 << 24) + 1, 
                // UDP parameters
                packetTime = 3 s, 
                packetSize = 10000, 
                nodename = "client 4", 
                nodenr = 4, 
                udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 3, 
                routingFile = "client4.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=205,207;i=comp";
    connections: 
        server1.out[0] --> client2.in[0]; 
        server1.out[1] --> client3.in[0]; 
        server1.out[2] --> client4.in[0]; 
                                          
        client2.out[0] --> server1.in[0]; 
        client2.out[1] --> client3.in[1]; 
        client2.out[2] --> client4.in[1]; 
                                          
        client3.out[0] --> server1.in[1]; 
        client3.out[1] --> client2.in[1]; 
        client3.out[2] --> client4.in[2]; 
                                          
        client4.out[0] --> server1.in[2]; 
        client4.out[1] --> client2.in[2]; 
        client4.out[2] --> client3.in[2]; 
                                          
                                          
    display: "p=-30,10;b=359,295,rect";
endmodule 

network directnw : TCPUDPDirectNet 
endnetwork 

