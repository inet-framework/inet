package inet.transport.rtp;

moduleinterface IRTCPModule
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}

simple RTCPModule like IRTCPModule
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}

simple RTCPEndsystemModule like IRTCPModule
{
    gates:
        input fromRTP;
        input fromUDP;
        output toRTP;
        output toUDP;
}
