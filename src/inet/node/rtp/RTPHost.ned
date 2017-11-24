// the original file was included in the ~UDP sockets example

package inet.node.rtp;

import inet.applications.rtpapp.RtpApplication;
import inet.node.inet.StandardHost;
import inet.transportlayer.rtp.Rtcp;
import inet.transportlayer.rtp.Rtp;
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
        rtpApp: RtpApplication {
            parameters:
                profileName = profileName;
                destinationAddress = destinationAddress;
                portNumber = portNumber;
                bandwidth = bandwidth;
                fileName = fileName;
                payloadType = payloadType;
                @display("p=736,60");
        }
        rtp: Rtp {
            parameters:
                @display("p=637,132");
        }
        rtcp: Rtcp {
            parameters:
                @display("p=838,132");
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

