package inet.transport.rtp;

//
// RTCP end system.
//
simple RTCP
{
    parameters:
        @display("i=block/control");
        @statistic[rcvdPkBytes](title="Received packet bytes"; record=count,sum,vector);
        @statistic[endToEndDelay](title="End to end delay"; record=stats,histogram,vector);

    gates:
        input rtpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output rtpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
