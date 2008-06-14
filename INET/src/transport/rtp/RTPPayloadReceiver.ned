package inet.transport.rtp;

simple RTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

//NOTE: like RTPPayloadReceiver
simple RTPAVProfilePayload10Receiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

//NOTE: like RTPPayloadReceiver
simple RTPAVProfilePayload32Receiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

//NOTE: like RTPPayloadReceiver
simple RTPAVProfileSampleBasedAudioReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}
