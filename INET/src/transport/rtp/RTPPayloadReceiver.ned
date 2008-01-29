package org.omnetpp.inet.transport.rtp;

simple RTPPayloadReceiver

{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;

}
