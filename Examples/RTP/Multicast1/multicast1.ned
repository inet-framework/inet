import
    "RTPHost", 
    "Router"; 
              
              
              
 
channel ethernet 
    delay normal(0.00015,0.00005); 
    datarate 10*10^6; 
endchannel 


module RTPMulticast1 
                     
    parameters:
        debug : bool; 
                      
    submodules: 
        host1: RTPHost; 
            parameters: 
                debug = debug, 
                nodename = "host1", 
                profileName = "RTPAVProfile", 
                destinationAddress = "225.0.0.1", 
                portNumber = 5004, 
                bandwidth = 8000, 
                numOfProcessors = 1,
                numOfPorts = 1, 
                routingFile = "host1.mrt"; 
            display: "p=150,50;i=pc";
        host2: RTPHost; 
            parameters: 
                debug = debug, 
                nodename = "host2", 
                profileName = "RTPAVProfile", 
                destinationAddress = "225.0.0.1", 
                portNumber = 5004, 
                bandwidth = 8000, 
                numOfProcessors = 1,
                numOfPorts = 1, 
                routingFile = "host2.mrt"; 
            display: "p=250,150;i=pc";
        host3: RTPHost; 
            parameters: 
                debug = debug, 
                nodename = "host3", 
                profileName = "RTPAVProfile", 
                destinationAddress = "225.0.0.1", 
                portNumber = 5004, 
                bandwidth = 8000, 
                numOfProcessors = 1,
                numOfPorts = 1, 
                routingFile = "host3.mrt"; 
            display: "p=150,250;i=pc";
        host4: RTPHost; 
            parameters: 
                debug = debug, 
                nodename = "host4", 
                profileName = "RTPAVProfile", 
                destinationAddress = "225.0.0.1", 
                portNumber = 5004, 
                bandwidth = 8000, 
                numOfProcessors = 1,
                numOfPorts = 1, 
                routingFile = "host4.mrt"; 
            display: "p=50,150;i=pc";
        router1: Router; 
            parameters: 
                numOfPorts = 4, 
                routingFile = "router1.mrt", 
                numOfProcessors = 1, 
                nodename = "router1"; 
            gatesizes: 
                in[4], 
                out[4]; 
                        
            display: "p=150,150;i=router";
    connections: 
        host1.out[0] --> ethernet --> router1.in[0]; 
        host1.in[0] <-- ethernet <-- router1.out[0]; 
        host2.out[0] --> ethernet --> router1.in[1]; 
        host2.in[0] <-- ethernet <-- router1.out[1]; 
        host3.out[0] --> ethernet --> router1.in[2]; 
        host3.in[0] <-- ethernet <-- router1.out[2]; 
        host4.out[0] --> ethernet --> router1.in[3]; 
        host4.in[0] <-- ethernet <-- router1.out[3]; 
                                                     
endmodule 

network rtpNetwork : RTPMulticast1 
    parameters: 
        debug = true; 
endnetwork 

