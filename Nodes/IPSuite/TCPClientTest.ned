// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers

import
    "Blackboard",
    "LinkLayer",
    "NetworkLayer",
    "mytcp4ip";



module TCPClientTest
    parameters:
        // TCP information
        start_time : numeric,
        local_addr : string,
        server_addr : string,
        // Network information
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=60,60";
        // transport and application layers
        tcpApp: MyTCPUpperLayers;
            parameters:
                mytcpApplType = "MyTCPClient",
                start_time = start_time,
                local_addr = local_addr,
                server_addr = server_addr;
            display: "p=84,69;b=40,24,rect";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
        netIf : LinkLayer[numOfPorts];
            parameters:
                NWIName = "PPPInterface";
            display: "p=80,220,row;i=iface";
    connections nocheck:
        // transport connections
        networkLayer.TCPOut --> tcpApp.in;
        networkLayer.TCPIn <-- tcpApp.out;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> netIf[i].physIn;
            out[i] <-- netIf[i].physOut;
            netIf[i].inputQueueOut --> networkLayer.physIn[i];
            netIf[i].outputQueueIn <-- networkLayer.physOut[i];
        endfor;
endmodule

