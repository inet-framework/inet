package inet.transport.rtp;

//
// RTCP end system.
//
simple RTCP
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}
