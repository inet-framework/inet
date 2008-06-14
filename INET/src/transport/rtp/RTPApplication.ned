package inet.transport.rtp;

simple RTPApplication
{
    parameters:
        @display("i=old/app");
        string commonName;
        string profileName;
        double bandwidth;
        string destinationAddress;
        int portNumber;
        string fileName;
        int payloadType;
        volatile double sessionEnterDelay;
        volatile double transmissionStartDelay;
        volatile double transmissionStopDelay;
        volatile double sessionLeaveDelay;

    gates:
        output toRTP;
        input fromRTP;
}
