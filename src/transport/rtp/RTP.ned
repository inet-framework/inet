package inet.transport.rtp;

//
// The \RTP module is the center of the \RTP layer of an endsystem.
// It communicates with the application, and sends and receives \RTP data
// packets.
//
// Much of the work is done by dynamically created Profile, and Payload
// Sender and Receiver modules (the latter two are created by Profile).
//
// See RTPProfile, RTPAVProfile
//
simple RTP
{
    parameters:
        @display("i=block/buffer");
    gates:
        input appIn @labels(RTPInterfacePacket/down);
        input profileIn @labels(RTPInnerPacket);
        input rtcpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output appOut @labels(RTPInterfacePacket/up);
        output profileOut @labels(RTPInnerPacket);
        output rtcpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}


