

import  
    "Router", 
    "UDPSocketHost";
                    
                    
 
module UDPSockets 
    submodules: 
        // server
        server1: UDPSocketHost; 
            parameters: 
                // UDP parameters
                //packetTime = 0.2 s, 
                //packetSize = 1000,
		client_name = "Server",
                nodename = "server 1", 
		//nodenr = 1, 
                //udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 1, 
                routingFile = "server1.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[1], 
                out[1];
            display: "p=40,83;i=comp";
        // clients
        client2: UDPSocketHost; 
            parameters: 
                // UDP parameters
		client_name = "Client",
		//packetTime = 0.5 s, 
                //packetSize = 1000, 
                nodename = "client 2", 
                //nodenr = 2, 
                //udpRouteDestNo = 1, 
                // network parameters
                numOfPorts = 1, 
                routingFile = "client2.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=200,83;i=comp";
    connections: 
        server1.out[0] --> client2.in[0]; 
        client2.out[0] --> server1.in[0]; 
                                          
                                          
    display: "p=-30,10;b=359,295,rect";
endmodule 

network udpsockets : UDPSockets 
endnetwork 

