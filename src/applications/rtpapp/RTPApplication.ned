package inet.applications.rtpapp;

simple RTPApplication
{
    parameters:
        @display("i=old/app");
        string commonName;
        string profileName;
        double bandwidth;  //XXX in bytes(!) per sec
        string destinationAddress;
        int portNumber;
        string fileName;
        int payloadType;
        double sessionEnterDelay @unit(s);
        double transmissionStartDelay @unit(s);
        double transmissionStopDelay @unit(s);
        double sessionLeaveDelay @unit(s);

    gates:
        output toRTP;
        input fromRTP;
}
