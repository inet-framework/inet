// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers




module TCPClientTest
{
    parameters:
        // TCP information
        volatile double start_time;
        string local_addr;
        string server_addr;
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
                mytcpApplType = "MyTCPClient";
                start_time = start_time;
                local_addr = local_addr;
                server_addr = server_addr;
                @display("p=84,69;b=40,24,rect");
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

