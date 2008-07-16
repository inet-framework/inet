package inet.examples.rtp.unicast;

import inet.transport.rtp.RTPHost;

network RTPNetwork
{
    types:
        channel ethernet extends ned.DatarateChannel
        {
            parameters:
                delay = normal(0.00015s, 0.00005s);
                datarate = 10Mbps;
                ber = 0.00001;
        }

    submodules:
        sender: RTPHost {
            parameters:
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "10.0.0.2";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "../Data/moving.mpg.gdf";
                payloadType = 32;
                routingFile = "server.irt";
        }
        receiver: RTPHost {
            parameters:
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "10.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "";
                payloadType = 32;
                routingFile = "client.irt";
        }
    connections:
        sender.pppg++ <--> ethernet <--> receiver.pppg++;
}
