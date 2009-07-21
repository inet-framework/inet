// the original file was included in the UDP sockets example

package inet.transport.rtp;

import inet.applications.rtpapp.RTPApplication;
import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.udp.UDP;


module RTPHost
{
    parameters:
        string profileName;
        string destinationAddress;
        int portNumber;
        double bandwidth;
        string fileName;
        int payloadType;
        // network information
        bool IPForward;
        string routingFile;
        @display("i=device/pc2");

    gates:
        inout pppg[] @labels(PPPFrame-conn);

    submodules:
        notificationBoard: NotificationBoard {
            @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            @display("p=60,134");
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
                profileName = profileName;
                destinationAddress = destinationAddress;
                portNumber = portNumber;
                bandwidth = bandwidth;
                fileName = fileName;
                payloadType = payloadType;
                @display("p=161,36");
        }
        rtpLayer: RTPLayer {
            parameters:
                @display("p=161,104");
        }
        udpModule: UDP {
            parameters:
                @display("p=161,170");
            gates:
                appIn[2];
                appOut[2];
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=161,246");
            gates:
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=161,324,row");
        }
    connections allowunconnected:
        // transport connections
        application.rtpOut --> rtpLayer.appIn;
        application.rtpIn <-- rtpLayer.appOut;

        rtpLayer.toUDPfromRTP --> udpModule.appIn[0];
        rtpLayer.fromUDPtoRTP <-- udpModule.appOut[0];

        rtpLayer.toUDPfromRTCP --> udpModule.appIn[1];
        rtpLayer.fromUDPtoRTCP <-- udpModule.appOut[1];

        networkLayer.udpOut --> udpModule.ipIn;
        networkLayer.udpIn <-- udpModule.ipOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

}

