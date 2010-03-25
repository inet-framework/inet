package inet.transport.rtp;

module RTPLayer
{
    parameters:
        @display("i=block/layer;bgb=178,105");

    gates:
        input appIn @labels(RTPInterfacePacket/down);
        input fromUDPtoRTP @labels(UDPControlInfo/up);
        input fromUDPtoRTCP @labels(UDPControlInfo/up);
        output appOut @labels(RTPInterfacePacket/up);
        output toUDPfromRTP @labels(UDPControlInfo/down);
        output toUDPfromRTCP @labels(UDPControlInfo/down);

    submodules:
        rtp: RTP {
            @display("p=63,64");
        }
        rtcp: RTCP {
            @display("p=135,64");
        }

    connections allowunconnected:
        appIn --> rtp.appIn;
        appOut <-- rtp.appOut;
        rtp.rtcpOut --> rtcp.rtpIn;
        rtp.rtcpIn <-- rtcp.rtpOut;
        rtp.udpOut --> toUDPfromRTP;
        rtp.udpIn <-- fromUDPtoRTP;
        rtcp.udpOut --> toUDPfromRTCP;
        rtcp.udpIn <-- fromUDPtoRTCP;

}


