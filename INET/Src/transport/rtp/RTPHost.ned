// the original file was included in the UDP sockets example



package org.omnetpp.inet.Transport.RTP;

import org.omnetpp.inet.NetworkInterfaces.PPP.PPPInterface;
import org.omnetpp.inet.Nodes.INET.NetworkLayer;


module RTPHost
{
    parameters:
        bool debug = default(false);
        string profileName;
        string destinationAddress = default("");
        int portNumber = default(554);
        double bandwidth @unit("bps");
        string fileName = default("");
        int payloadType = default(0);
        // network information
        int numOfProcessors = default(1);
        string routingFile = default("");
        @display("i=device/pc2");
    gates:
        inout pppg[];

    submodules:
        application: RTPApplication {
            parameters:
                //#commonName = nodename,
                profileName = profileName;
                destinationAddress = destinationAddress;
                portNumber = portNumber;
                bandwidth = bandwidth;
                fileName = fileName;
                payloadType = payloadType;
                @display("p=77,52;b=40,24");
        }
        rtpLayer: RTPLayer {
            parameters:
                rtpModuleType = "RTPEndsystemModule";
                rtcpModuleType = "RTCPEndsystemModule";
                @display("p=178,54;b=40,24");
        }
        socketLayer: SocketLayer {
            parameters:
                debug = false;
                @display("p=180,164;b=40,24");
            gates:
                from_appl[2];
                to_appl[2];
        }
        udpModule: UDPModule {
            parameters:
                debug = false;
                @display("p=87,166");
        }
        networkLayer: NetworkLayer {
            parameters:
                IPForward = false;
                routingFile = routingFile;
                @display("p=87,242");
            gates:
                ifIn[sizeof(out)];
                ifOut[sizeof(out)];
        }
        ppp[sizeof(out)]: PPPInterface {
            parameters:
                @display("p=80,300,row;q=queue");
        }
    connections allowunconnected:
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
        for i=0..sizeof(out)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }
}

