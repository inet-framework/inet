package inet.transport.rtp;

//
// RTCP end system.
//
simple RTCP
{
    parameters:
        @display("i=block/control");
    gates:
        input rtpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output rtpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
