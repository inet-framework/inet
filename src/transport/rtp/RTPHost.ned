// the original file was included in the ~UDP sockets example

package inet.transport.rtp;

import ned.IdealChannel;
import inet.transport.udp.UDP;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWiredNic;
import inet.base.NotificationBoard;
import inet.applications.rtpapp.RTPApplication;


module RTPHost
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        string profileName;
        string destinationAddress;
        int portNumber;
        double bandwidth;
        string fileName;
        int payloadType;
        // network information
        bool IPForward = default(false);
        string routingFile = default("");
        @display("i=device/pc2;bgb=323,371");

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
        rtp: RTP {
            @display("p=163,104");
        }
        rtcp: RTCP {
            @display("p=243,104");
        }
        udp: UDP {
            parameters:
                @display("p=202,173");
            gates:
                appIn[2];
                appOut[2];
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=202,244");
            gates:
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=202,320,row");
        }

    connections allowunconnected:
        // transport connections
        application.rtpOut --> rtp.appIn;
        application.rtpIn <-- rtp.appOut;

        rtp.udpOut --> udp.appIn[0];
        rtp.udpIn <-- udp.appOut[0];

        rtcp.udpOut --> udp.appIn[1];
        rtcp.udpIn <-- udp.appOut[1];

        networkLayer.udpOut --> udp.ipIn;
        networkLayer.udpIn <-- udp.ipOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }

        rtp.rtcpOut --> IdealChannel --> rtcp.rtpIn;
        rtcp.rtpOut --> IdealChannel --> rtp.rtcpIn;
}

