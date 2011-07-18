package inet.transport.rtp;

//
// RTCP end system.
//
simple RTCP
{
    parameters:
        @display("i=block/control");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPkBytes](title="Received packet bytes"; source="packetBytes(rcvdPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[endToEndDelay](title="End to end delay"; source="messageAge(rcvdPk)"; unit=s; record=stats,histogram,vector; interpolationmode=none);

    gates:
        input rtpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output rtpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
