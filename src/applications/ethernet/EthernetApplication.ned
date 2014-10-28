package inet.applications.ethernet;

//******************************************************
// @file EthernetApplication.cc
// @brief Simple traffic generator.
// It generates Etherapp requests and responses. Based in EtherAppCli and EtherAppSrv.
//
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//
//
//******************************************************/
simple EthernetApplication
{
    parameters:
        string destAddress = default("");  // destination MAC address, or module path name of destination station; empty means off
        double startTime @unit("s") = default(this.waitTime);  // time to send the first request
        volatile double waitTime @unit("s") = default(uniform(0s,1s));  // interval between sending requests
        volatile int reqLength @unit(B) = default(100B);  // length of request packets
        volatile int respLength @unit(B) = default(1KiB);  // length of response packets
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
     	@display("i=block/app");
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
}
