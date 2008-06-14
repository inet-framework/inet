package inet.transport.rtp;

simple RTCPModule
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}

//NOTE: like RTCPModule
simple RTCPEndsystemModule
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}
