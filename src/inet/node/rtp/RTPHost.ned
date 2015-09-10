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
                @display("p=254,42");
        }
        rtp: RTP {
            @display("p=254,111");
        }
        rtcp: RTCP {
            @display("p=391,111");
        }

    connections allowunconnected:
        // transport connections
        rtpApp.rtpOut --> rtp.appIn;
        rtpApp.rtpIn <-- rtp.appOut;

        rtp.udpOut --> udp.appIn++;
        rtp.udpIn <-- udp.appOut++;

        rtcp.udpOut --> udp.appIn++;
        rtcp.udpIn <-- udp.appOut++;

        rtp.rtcpOut --> IdealChannel --> rtcp.rtpIn;
        rtcp.rtpOut --> IdealChannel --> rtp.rtcpIn;
}

