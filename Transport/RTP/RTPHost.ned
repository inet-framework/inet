// the original file was included in the UDP sockets example

import  
    "NetworkLayers", 
    "ProcessorManager", 
    "UDP", 
    "ControlApp", 
    "UDPModule",
    "socketlayer", 
    "RTPLayer", 
    "RTPApplication"; 
                      
 
module RTPHost

    parameters:
        debug: bool,
        nodename : string,
        profileName : string, 
        destinationAddress : string, 
        portNumber : numeric, 
        bandwidth : numeric, 
        fileName : string, 
        payloadType : numeric, 
        // network information
        numOfProcessors : numeric,
        numOfPorts : numeric,
        routingFile : string;
    gates: 
        in: in[];
        out: out[];
                    
    submodules: 
        application: RTPApplication; 
            parameters:
                commonName = nodename,
                profileName = profileName, 
                destinationAddress = destinationAddress, 
                portNumber = portNumber, 
                bandwidth = bandwidth, 
                fileName = fileName, 
                payloadType = payloadType;
            display: "p=77,52;b=40,24";
        rtpLayer: RTPLayer; 
            parameters:
                rtpModuleType = "RTPEndsystemModule",
                rtcpModuleType = "RTCPEndsystemModule"; 
            display: "p=178,54;b=40,24";
        socketLayer: SocketLayer;
            parameters:
                debug = false;
            gatesizes:
                from_appl[2],
                to_appl[2];
            display: "p=180,164;b=40,24";
        udpModule: UDPModule;
            parameters:
                debug = false,
                nodename = nodename;
            display: "b=32,30;p=87,166;i=prot2";
        errorHandling: ErrorHandling;
            parameters:
                nodename = routingFile;

            display: "p=209,238;b=40,24";
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
                NWIName = "PPPModule",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "b=32,30;p=87,242;i=fork";
        processorManager: ProcessorManager;
            parameters:
                numOfProcessors = numOfProcessors;
            display: "b=34,34;p=84,106;i=bwgen";
    connections nocheck: 
        // transport connections
        application.toRTP --> rtpLayer.fromApp;
        application.fromRTP <-- rtpLayer.toApp; 
        rtpLayer.toSocketLayerRTP --> socketLayer.from_appl[0];
        rtpLayer.fromSocketLayerRTP <-- socketLayer.to_appl[0];
        rtpLayer.toSocketLayerRTCP --> socketLayer.from_appl[1];
        rtpLayer.fromSocketLayerRTCP <-- socketLayer.to_appl[1];
        socketLayer.to_udp --> udpModule.from_socket;
        socketLayer.from_udp <-- udpModule.to_socket;
        networkLayers.UDPOut --> udpModule.from_ip;
        networkLayers.UDPIn <-- udpModule.to_ip;
        networkLayers.errorOut --> errorHandling.in;
                                                     
        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
        endfor;
endmodule 

