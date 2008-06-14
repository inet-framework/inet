// the original file was included in the UDP sockets example

import NotificationBoard;
import NetworkLayer;
import PingApp;
import InterfaceTable;
import RoutingTable;
import UDP;
import RTPLayer;
import RTPApplication;
import EthernetInterface;

module RTPHost
{
    parameters:
        bool debug;
        string profileName;
        string destinationAddress;
        int portNumber;
        double bandwidth;
        string fileName;
        int payloadType;
        // network information
        int numOfProcessors;
        bool IPForward;
        double namid;
        string routingFile;
        @display("bgb=212,371");
    gates:
        input in[];
        output out[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,134;i=block/table");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,214");
        }
        application: RTPApplication {
            parameters:
                //#commonName = nodename,
                profileName = profileName;
                destinationAddress = destinationAddress;
                portNumber = portNumber;
                bandwidth = bandwidth;
                fileName = fileName;
                payloadType = payloadType;
                @display("b=,,,,,2;i=old/app;p=161,36");
        }
        rtpLayer: RTPLayer {
            parameters:
                rtpModuleType = "RTPEndsystemModule";
                rtcpModuleType = "RTCPEndsystemModule";
                @display("p=161,104");
        }
        udpModule: UDP {
            parameters:
                @display("p=161,170");
            gates:
                from_app[2];
                to_app[2];
        }
        networkLayer: NetworkLayer {
            parameters:
                //debug = false,
                proxyARP = false;
                @display("p=161,246");
            gates:
                ifIn[sizeof(out)];
                ifOut[sizeof(out)];
        }
        ppp[sizeof(out)]: PPPInterface {
            parameters:
                @display("p=161,324,row");
        }
    connections allowunconnected:
        // transport connections
        application.toRTP --> rtpLayer.fromApp;
        application.fromRTP <-- rtpLayer.toApp;

        rtpLayer.toUDPfromRTP --> udpModule.from_app[0];
        rtpLayer.fromUDPtoRTP <-- udpModule.to_app[0];

        rtpLayer.toUDPfromRTCP --> udpModule.from_app[1];
        rtpLayer.fromUDPtoRTCP <-- udpModule.to_app[1];

        networkLayer.UDPOut --> udpModule.from_ip;
        networkLayer.UDPIn <-- udpModule.to_ip;

        // connections to network outside
        for i=0..sizeof(out)-1 {
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

}

