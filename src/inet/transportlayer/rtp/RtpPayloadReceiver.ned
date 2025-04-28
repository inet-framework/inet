//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

import inet.common.SimpleModule;

//
// Interface for RTP Payload Receivers.
//
// These modules are internal to the ~Rtp module; instances are created
// and connected dynamically by ~RtpProfile.
//
moduleinterface IRTPPayloadReceiver
{
    parameters:
        string outputFileName;  // Use an empty string to disable this logging
        string outputLogFileName;   // Use an empty string to disable this logging

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpPayloadReceiver extends SimpleModule like IRTPPayloadReceiver
{
    parameters:
        @class(RtpPayloadReceiver);
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");
        @signal[rcvdPkRtpTimestamp](type=simtime_t; unit=s);
        @statistic[rcvdPkRtpTimestamp](title="received packet RTP timestamp"; record=vector);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Receiver extends SimpleModule like IRTPPayloadReceiver
{
    parameters:
        @class(RTPAVProfilePayload10Receiver);
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Receiver extends SimpleModule like IRTPPayloadReceiver
{
    parameters:
        @class(RtpAvProfilePayload32Receiver);
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioReceiver extends SimpleModule like IRTPPayloadReceiver
{
    parameters:
        @class(RTPAVProfileSampleBasedAudioReceiver);
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

