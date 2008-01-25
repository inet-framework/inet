package org.omnetpp.inet.Transport.RTP;

simple RTPProfile

{
    parameters:
        string autoOutputFileNames;

    gates:
        input fromRTP;
        input fromPayloadSender;
        input fromPayloadReceiver[];
        output toRTP;
        output toPayloadSender;
        output toPayloadReceiver[];

}
