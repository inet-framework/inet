// from MPLS models -- FIXME merge or eliminate

// Definition of an IPNode with transport layers

import
    "NetworkLayers",
    //"ProcessorManager",
    "mytcp4ip",
    "ErrorHandling";



module TCPServerTest
    parameters:
        // TCP information
        local_addr : string,
        // Network information
        numOfProcessors : numeric,
        numOfPorts : numeric,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        // transport and application layers
        tcpApp: MyTCPUpperLayers;
            parameters:
               start_time =0,
                mytcpApplType = "MyTCPServer",
                local_addr = local_addr,
                server_addr = "0.0.0.0";
            display: "p=84,61;b=40,24,rect";
        errorHandling: ErrorHandling;
            parameters:
                nodename = routingFile;
            display: "p=179,158;i=sink";
        networkLayers: NetworkLayers;
            parameters:
                // waiting times
                icmp_delay = 0,
                fragmentation_delay = 0.2 us,
                multicast_delay = 0.5 us,
                ipoutput_delay = 0,
                ipsend_delay = 0,
                iptunneling_delay = 0,
                localdeliver_delay = 0,
                prerouting_delay = 0.2 us,
                routing_delay = 1 us,
                enqueue_delay = 0,
                dequeue_delay = 0,
                inputqueue_delay = 0,
                nwi_delay = 0,

                // hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                // other network parameters
                IPForward = false,
                NWIName = "MPLS2IP",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "p=87,155;i=fork";
        //# processorManager: ProcessorManager;
    connections nocheck:
        // transport connections
        networkLayers.TCPOut --> tcpApp.in;
        networkLayers.TCPIn <-- tcpApp.out;

        networkLayers.errorOut --> errorHandling.in;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;

endmodule

