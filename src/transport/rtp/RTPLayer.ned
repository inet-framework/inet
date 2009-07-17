package inet.transport.rtp;

module RTPLayer
{
    parameters:
        @display("i=block/layer;bgb=178,105");

    gates:
        input fromApp;
        input fromUDPtoRTP @labels(UDPControlInfo/up);
        input fromUDPtoRTCP @labels(UDPControlInfo/up);
        output toApp;
        output toUDPfromRTP @labels(UDPControlInfo/down);
        output toUDPfromRTCP @labels(UDPControlInfo/down);

    submodules:
        rtp: RTP {
            @display("i=block/buffer;p=63,64");
        }
        rtcp: RTCP {
            @display("i=block/control;p=135,64");
        }

    connections allowunconnected:
        fromApp --> rtp.fromApp;
        toApp <-- rtp.toApp;
        rtp.toRTCP --> rtcp.fromRTP;
        rtp.fromRTCP <-- rtcp.toRTP;
        rtp.toUDPLayer --> toUDPfromRTP;
        rtp.fromUDPLayer <-- fromUDPtoRTP;
        rtcp.toUDP --> toUDPfromRTCP;
        rtcp.fromUDP <-- fromUDPtoRTCP;

}


