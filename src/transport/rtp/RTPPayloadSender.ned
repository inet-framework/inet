package inet.transport.rtp;

//
// Interface for RTP Payload Senders.
//
// These modules are internal to the ~RTP module; instances are created
// and connected dynamically by ~RTPProfile.
//
moduleinterface IRTPPayloadSender
{
    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPPayloadSender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfilePayload10Sender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfilePayload32Sender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfileSampleBasedAudioSender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

