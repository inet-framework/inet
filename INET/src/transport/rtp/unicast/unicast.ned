package inet.transport.rtp.unicast;

import inet.transport.rtp.RTPHost;

network RTPNetwork
{
    parameters:
        bool debug = default(true);
        
    types:
        channel ethernet extends ned.BasicChannel
        {
            parameters:
                delay = normal(0.00015s, 0.00005s);
                datarate = 10Mbps;
        }
    
    submodules:
        sender: RTPHost {
            parameters:
                debug = debug;
                IPForward = false;
                profileName = "RTPAVProfile";
                destinationAddress = "10.0.0.2";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "../Data/moving.mpg.gdf";
                payloadType = 32;
                numOfProcessors = 1;
                routingFile = "server.irt";
                @display("p=50,50;i=device/pc2");
        }
        receiver: RTPHost {
            parameters:
                debug = debug;
                IPForward = false;
                profileName = "RTPAVProfile";
                destinationAddress = "10.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "";
                payloadType = 32;
                numOfProcessors = 1;
                routingFile = "client.irt";
                @display("p=150,50;i=device/pc2");
        }
    connections:
        sender.pppg++ <--> ethernet <--> receiver.pppg++;
}
