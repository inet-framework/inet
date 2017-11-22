package inet.transportlayer.rtp;

//
// Interface for RTP Payload Senders.
//
// These modules are internal to the ~Rtp module; instances are created
// and connected dynamically by ~RtpProfile.
//
moduleinterface IRTPPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpPayloadSender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Sender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Sender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioSender like IRTPPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

