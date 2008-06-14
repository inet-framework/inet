package inet.transport.rtp;

simple RTPPayloadSender
{
    gates:
        input fromProfile;
        output toProfile;
}

//NOTE: like RTPPayloadSender
simple RTPAVProfilePayload10Sender
{
    gates:
        input fromProfile;
        output toProfile;
}

//NOTE: like RTPPayloadSender
simple RTPAVProfilePayload32Sender
{
    gates:
        input fromProfile;
        output toProfile;
}


//NOTE: like RTPPayloadSender
simple RTPAVProfileSampleBasedAudioSender
{
    gates:
        input fromProfile;
        output toProfile;
}

