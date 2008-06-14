package inet.transport.rtp;

moduleinterface IRTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

simple RTPPayloadReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfilePayload10Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfilePayload32Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfileSampleBasedAudioReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;

    gates:
        input fromProfile;
        output toProfile;
}
