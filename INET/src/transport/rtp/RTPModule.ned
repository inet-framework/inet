package org.omnetpp.inet.transport.rtp;

simple RTPModule

{
    gates:
        input fromApp;
        input fromProfile;
        input fromRTCP;
        input fromSocketLayer;
        output toApp;
        output toProfile;
        output toRTCP;
        output toSocketLayer;

}
