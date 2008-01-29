package org.omnetpp.inet.transport.rtp;

simple RTCPModule

{
    gates:
        input fromRTP;
        input fromSocketLayer;
        output toRTP;
        output toSocketLayer;

}
