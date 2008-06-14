package inet.transport.rtp;

moduleinterface IRTPModule
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

simple RTPModule like IRTPModule
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

simple RTPEndsystemModule like IRTPModule
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
