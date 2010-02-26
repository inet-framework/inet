package inet.transport.rtp;

//
// RTCP end system.
//
simple RTCP
{
    parameters:
        @display("i=block/control");
        @signal[rcvdPkBytes](title="Received packet bytes"; modeHint=count,sum);
        @signal[endToEndDelay](title="End to end delay"; modeHint=stats,histogram);

    gates:
        input rtpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output rtpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
