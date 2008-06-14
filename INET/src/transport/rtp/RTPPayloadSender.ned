package inet.transport.rtp;

moduleinterface IRTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

simple RTPPayloadSender like IRTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfilePayload10Sender like IRTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfilePayload32Sender like IRTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

simple RTPAVProfileSampleBasedAudioSender like IRTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

