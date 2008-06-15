package inet.transport.rtp;

simple RTPProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input fromRTP;
        input fromPayloadSender;
        input fromPayloadReceiver[];
        output toRTP;
        output toPayloadSender;
        output toPayloadReceiver[];
}

//NOTE: like RTPProfile
simple RTPAVProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input fromRTP;
        input fromPayloadSender;
        input fromPayloadReceiver[];
        output toRTP;
        output toPayloadSender;
        output toPayloadReceiver[];
}
