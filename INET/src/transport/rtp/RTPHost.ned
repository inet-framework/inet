// the original file was included in the UDP sockets example



package inet.transport.rtp;

import inet.networkinterfaces.ppp.PPPInterface;
import inet.nodes.inet.NetworkLayer;


module RTPHost
{
    parameters:
        @node();
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
                appIn[2];
                appOut[2];
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
        rtpLayer.toSocketLayerRTP --> socketLayer.appIn[0];
        rtpLayer.fromSocketLayerRTP <-- socketLayer.appOut[0];
        rtpLayer.toSocketLayerRTCP --> socketLayer.appIn[1];
        rtpLayer.fromSocketLayerRTCP <-- socketLayer.appOut[1];
        socketLayer.udpOut --> udpModule.from_socket;
        socketLayer.udpIn <-- udpModule.to_socket;
        networkLayer.udpOut --> udpModule.ipIn;
        networkLayer.udpIn <-- udpModule.ipOut;

        // connections to network outside
        for i=0..sizeof(out)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }
}

