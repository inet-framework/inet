//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

//
// Interface for RTP Payload Receivers.
//
// These modules are internal to the ~Rtp module; instances are created
// and connected dynamically by ~RtpProfile.
//
moduleinterface IRTPPayloadReceiver
{
    parameters:
        string outputFileName;  // use an empty string to disable this logging
        string outputLogFileName;   // use an empty string to disable this logging

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpPayloadReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");
        @signal[rcvdPkRtpTimestamp](type=simtime_t; unit=s);
        @statistic[rcvdPkRtpTimestamp](title="received packet RTP timestamp"; record=vector);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

