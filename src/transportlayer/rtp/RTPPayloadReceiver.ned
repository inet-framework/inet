package inet.transport.rtp;

//
// Interface for RTP Payload Receivers.
//
// These modules are internal to the ~RTP module; instances are created
// and connected dynamically by ~RTPProfile.
//
moduleinterface IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName;

    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPPayloadReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");
        @signal[rcvdPkRtpTimestamp](type=simtime_t;unit=s);
        @statistic[rcvdPkRtpTimestamp](title="received packet RTP timestamp"; record=vector);

    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfilePayload10Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfilePayload32Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}

simple RTPAVProfileSampleBasedAudioReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RTPInnerPacket);
        output profileOut @labels(RTPInnerPacket);
}


