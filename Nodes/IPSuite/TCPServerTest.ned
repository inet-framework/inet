// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers

import
    "Blackboard",
    "NetworkLayer",
    "mytcp4ip";



module TCPServerTest
    parameters:
        // TCP information
        local_addr : string,
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
               start_time =0,
                mytcpApplType = "MyTCPServer",
                local_addr = local_addr,
                server_addr = "0.0.0.0";
            display: "p=84,61;b=40,24,rect";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
        ppp : PPPInterface[numOfPorts];
            display: "p=80,220,row;i=iface";
    connections nocheck:
        // transport connections
        networkLayer.TCPOut --> tcpApp.in;
        networkLayer.TCPIn <-- tcpApp.out;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;
endmodule

