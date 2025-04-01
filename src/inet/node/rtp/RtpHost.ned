//
// Copyright (C) 2020 OpenSim Ltd.
// the original file was included in the ~Udp sockets example

package inet.node.rtp;

import inet.applications.rtpapp.RtpApplication;
import inet.node.inet.StandardHost;
import inet.transportlayer.rtp.Rtcp;
import inet.transportlayer.rtp.Rtp;
import ned.IdealChannel;

//
// A network node with Real-time Transport Protocol (RTP) support.
// Extends StandardHost by adding RTP-specific components for real-time multimedia
// streaming applications.
//
// RTP provides end-to-end delivery services for data with real-time characteristics,
// such as interactive audio and video. It works alongside the RTP Control Protocol (RTCP)
// which monitors transmission statistics and quality of service.
//
module RtpHost extends StandardHost
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
                profileName = parent.profileName;
                destinationAddress = parent.destinationAddress;
                portNumber = parent.portNumber;
                bandwidth = parent.bandwidth;
                fileName = parent.fileName;
                payloadType = parent.payloadType;
                @display("p=825,76");
        }
        rtp: Rtp {
            parameters:
                @display("p=975,76");
        }
        rtcp: Rtcp {
            parameters:
                @display("p=1125,76");
        }

    connections allowunconnected:
        // transport connections
        rtpApp.rtpOut --> rtp.appIn;
        rtpApp.rtpIn <-- rtp.appOut;

        rtp.udpOut --> at.in++;
        rtp.udpIn <-- at.out++;

        rtcp.udpOut --> at.in++;
        rtcp.udpIn <-- at.out++;

        rtp.rtcpOut --> IdealChannel --> rtcp.rtpIn;
        rtcp.rtpOut --> IdealChannel --> rtp.rtcpIn;
}

