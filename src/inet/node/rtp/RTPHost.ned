// the original file was included in the ~UDP sockets example

package inet.node.rtp;

import inet.applications.rtpapp.RTPApplication;
import inet.node.inet.StandardHost;
import inet.transportlayer.rtp.RTCP;
import inet.transportlayer.rtp.RTP;
import ned.IdealChannel;


//
// TODO document!
//
module RTPHost extends StandardHost
{
    parameters:
        string profileName;
        string destinationAddress;
        int portNumber;
        double bandwidth;
        string fileName;
        int payloadType;
        hasUdp = true;
        @display("i=device/pc2");

    submodules:
        rtpApp: RTPApplication {
            parameters:
                profileName = profileName;
                destinationAddress = destinationAddress;
                portNumber = portNumber;
                bandwidth = bandwidth;
                fileName = fileName;
                payloadType = payloadType;
                @display("p=247,69");
        }
        rtp: RTP {
            parameters:
                @display("p=270,169");
        }
        rtcp: RTCP {
            parameters:
                @display("p=325,169");
        }

    connections allowunconnected:
        // transport connections
        rtpApp.rtpOut --> rtp.appIn;
        rtpApp.rtpIn <-- rtp.appOut;

        rtp.udpOut --> at.upperLayerIn++;
        rtp.udpIn <-- at.upperLayerOut++;

        rtcp.udpOut --> at.upperLayerIn++;
        rtcp.udpIn <-- at.upperLayerOut++;

        rtp.rtcpOut --> IdealChannel --> rtcp.rtpIn;
        rtcp.rtpOut --> IdealChannel --> rtp.rtcpIn;
}

