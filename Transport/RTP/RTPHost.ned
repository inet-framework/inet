// the original file was included in the UDP sockets example

import
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
            display: "p=87,166;i=block/transport";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=87,242;i=block/fork";
        ppp : PPPInterface[sizeof(out)];
            display: "p=80,300,row;q=queue;i=block/ifcard";
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
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;
endmodule

