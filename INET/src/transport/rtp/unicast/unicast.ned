import RTPHost;

channel ethernet extends ned.BasicChannel
{
    parameters:
        delay = normal(0.00015, 0.00005);
        datarate = 10*10^6;
}

module RTPNetwork

{
    parameters:
        bool debug;

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
            gates:
                in[1];
                out[1];
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
            gates:
                in[1];
                out[1];
        }
    connections:
        sender.out[0] --> ethernet --> receiver.in[0];
        sender.in[0] <-- ethernet <-- receiver.out[0];

}

network rtpNetwork extends RTPNetwork
{
    parameters:
        debug = true;
}
