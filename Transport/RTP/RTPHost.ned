// the original file was included in the UDP sockets example

import
    "LinkLayer",
    "NetworkLayer",
    "UDP",
    "UDPModule",
    "socketlayer",
    "RTPLayer",
    "RTPApplication";


module RTPHost

    parameters:
        debug: bool,
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
                //#commonName = nodename,
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
                debug = false;
            display: "b=32,30;p=87,166;i=prot2";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "b=32,30;p=87,242;i=fork";
        netIf : LinkLayer[numOfPorts];
            parameters:
                NWIName = "PPPModule";
            display: "p=80,300,row;b=38,24";
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
        networkLayer.UDPOut --> udpModule.from_ip;
        networkLayer.UDPIn <-- udpModule.to_ip;

        // connections to network outside
        for i=0..numOfPorts-1 do
            in[i] --> netIf[i].physIn;
            out[i] <-- netIf[i].physOut;
            netIf[i].inputQueueOut --> networkLayer.physIn[i];
            netIf[i].outputQueueIn <-- networkLayer.physOut[i];
        endfor;
endmodule

