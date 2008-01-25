package org.omnetpp.inet.Transport.RTP;

simple RTPApplication
{
    parameters:
        string commonName;
        string profileName;
        double bandwidth @unit("bps");
        string destinationAddress = default("");
        int portNumber;
        string fileName;
        int payloadType;
        volatile double sessionEnterDelay @unit("s");
        volatile double transmissionStartDelay @unit("s");
        volatile double transmissionStopDelay @unit("s");
        volatile double sessionLeaveDelay @unit("s");
    gates:
        output toRTP;
        input fromRTP;
}
