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

module McNetwork2 
    submodules: 
                // *** Hosts ***
                
        // Host 1
        host1: UDPHost; 
            parameters: 
                // network parameters
                IPForward = false, 
                numOfPorts = 1, 
                routingFile = "mchost1.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 0.9 s, 
                packetSize = 100000, 
                nodenr = 1, 
                nodename = "host 1"; 
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=62,46;i=pc";
        // Host 2
        host2: UDPHost; 
            parameters: 
                // network parameters
                IPForward = true, 
                numOfPorts = 2, 
                routingFile = "mchost2.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 0.2 s, 
                packetSize = 10000, 
                nodenr = 2, 
                nodename = "host 2"; 
            gatesizes: 
                in[2], 
                out[2]; 
                        
            display: "p=38,198;i=pc";
        // Host 3
        host3: UDPHost; 
            parameters: 
                // network parameters
                IPForward = true, 
                numOfPorts = 2, 
                routingFile = "mchost3.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 0.3 s, 
                packetSize = 20000, 
                nodenr = 3, 
                nodename = "host 3"; 
            gatesizes: 
                in[2], 
                out[2]; 
                        
            display: "p=140,197;i=pc";
        // Host 4
        host4: UDPHost; 
            parameters: 
                // network parameters
                IPForward = false, 
                numOfPorts = 1, 
                routingFile = "mchost4.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 0.5 s, 
                packetSize = 30000, 
                nodenr = 4, 
                nodename = "host 4"; 
            gatesizes: 
                in[1], 
                out[1]; 
                        
            display: "p=219,269;i=pc";
        // Host 5
        host5: UDPHost; 
            parameters: 
                // network parameters
                IPForward = false, 
                numOfPorts = 1, 
                routingFile = "mchost5.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 0.3 s, 
                packetSize = 20000, 
                nodenr = 5, 
                nodename = "host 5"; 
            gatesizes: 
                in[1], 
                out[1]; 
                        
            display: "p=344,41;i=pc";
        // Host 6
        host6: UDPHost; 
            parameters: 
                // network parameters
                IPForward = false, 
                numOfPorts = 1, 
                routingFile = "mchost6.mrt", 
                numOfProcessors = 1, 
                // UDP Transport parameters
                udpRouteDestNo = 3, 
                packetTime = 1.1 s, 
                packetSize = 8000, 
                nodenr = 6, 
                nodename = "host 6"; 
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=364,189;i=pc";
        // *** Router ****
        // Router 1
        router1: Router; 
            parameters: 
                // network parameters
                numOfPorts = 4, 
                routingFile = "mcrouter1.mrt", 
                numOfProcessors = 1, 
                nodename = "router 1"; 
            gatesizes: 
                in[4], 
                out[4]; 
                        
            display: "p=68,115;i=ipc";
        // Router 2
        router2: Router; 
            parameters: 
                // network parameters
                numOfPorts = 3, 
                routingFile = "mcrouter2.mrt", 
                numOfProcessors = 1, 
                nodename = "router 2"; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=214,113;i=router";
        // Router 3
        router3: Router; 
            parameters: 
                // network parameters
                numOfPorts = 2, 
                routingFile = "mcrouter3.mrt", 
                numOfProcessors = 1, 
                nodename = "router 3"; 
            gatesizes: 
                in[2], 
                out[2]; 
            display: "p=212,194;i=pc";
        // Router 4
        router4: Router; 
            parameters: 
                // network parameters
                numOfPorts = 3, 
                routingFile = "mcrouter4.mrt", 
                numOfProcessors = 1, 
                nodename = "router 4"; 
            gatesizes: 
                in[3], 
                out[3]; 
            display: "p=299,116;i=ipc";
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
                                                          
    display: "p=10,10;b=390,309,rect";
endmodule 

network mcNetwork2 : McNetwork2 
endnetwork 

