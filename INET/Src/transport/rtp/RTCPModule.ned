package org.omnetpp.inet.Transport.RTP;

simple RTCPModule

{
    gates:
        input fromRTP;
        input fromSocketLayer;
        output toRTP;
        output toSocketLayer;

}
