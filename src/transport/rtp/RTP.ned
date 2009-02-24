package inet.transport.rtp;

//
// RTP end system
//
simple RTP
{
    gates:
        input fromApp;
        input fromProfile;
        input fromRTCP;
        input fromUDPLayer;
        output toApp;
        output toProfile;
        output toRTCP;
        output toUDPLayer;
}
