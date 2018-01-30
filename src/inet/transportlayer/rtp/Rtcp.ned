package inet.transportlayer.rtp;

//
// RTCP end system.
//
simple Rtcp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/control");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input rtpIn @labels(RtpInnerPacket);
        input udpIn @labels(UdpControlInfo/up);
        output rtpOut @labels(RtpInnerPacket);
        output udpOut @labels(UdpControlInfo/down);
}
