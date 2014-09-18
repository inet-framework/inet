package inet.transportlayer.rtp;

//
// RTCP end system.
//
simple RTCP
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/control");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input rtpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output rtpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
