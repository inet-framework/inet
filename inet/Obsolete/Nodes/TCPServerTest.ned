// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers




module TCPServerTest
{
    parameters:
        // TCP information
        string local_addr;
        // Network information
        string routingFile;
    gates:
        input in[];
        output out[];
    submodules:
        blackboard: Blackboard {
            parameters:
                @display("p=60,60");
        };
        // transport and application layers
        tcpApp: MyTCPUpperLayers {
            parameters:
                start_time = 0;
                mytcpApplType = "MyTCPServer";
                local_addr = local_addr;
                server_addr = "0.0.0.0";
                @display("p=84,61;b=40,24,rect");
        };
        networkLayer: NetworkLayer {
            parameters:
                IPForward = false;
                routingFile = routingFile;
                @display("p=87,155");
            gates:
                physIn[sizeof(out)];
                physOut[sizeof(out)];
        };
        ppp[sizeof(out)]: PPPInterface {
            parameters:
                @display("p=80,220,row;q=queue");
        };
    connections allowunconnected:
        // transport connections
        networkLayer.TCPOut --> tcpApp.in;
        networkLayer.TCPIn <-- tcpApp.out;

        // connections to network outside
        for i=0..sizeof(out)-1 {
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        };
}

