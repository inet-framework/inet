// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers

import
    "NetworkLayers",
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
        // transport and application layers
        tcpApp: MyTCPUpperLayers;
            parameters:
                mytcpApplType = "MyTCPClient",
                start_time = start_time,
                local_addr = local_addr,
                server_addr = server_addr;
            display: "p=84,69;b=40,24,rect";
        networkLayers: NetworkLayers;
            parameters:
                IPForward = false,
                NWIName = "MPLS2IP",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
    connections nocheck:
        // transport connections
        networkLayers.TCPOut --> tcpApp.in;
        networkLayers.TCPIn <-- tcpApp.out;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

