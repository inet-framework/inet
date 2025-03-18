//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
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

//
// Base module for RTP payload receivers that process incoming RTP data packets.
// Provides common functionality such as opening and closing output files,
// handling incoming packets, and recording statistics. Serves as the foundation
// for specific payload type implementations.
//
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

//
// Specialized RTP payload receiver for PCM audio data (payload type 10) in the
// RTP audio/video profile. Processes incoming RTP packets containing PCM audio
// samples and writes them to an output file for playback or analysis.
//
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

//
// Specialized RTP payload receiver for MPEG video data (payload type 32) in the
// RTP audio/video profile. Processes incoming RTP packets containing MPEG video
// frames, performs packet reordering, and writes the data to an output file.
// Corresponds to the RtpAvProfilePayload32Sender module.
//
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

//
// Base class for RTP payload receivers that handle sample-based audio formats in
// the RTP audio/video profile. Provides functionality for processing audio
// samples from RTP packets, handling timing information, and writing the
// received audio data to output files for playback or analysis.
//
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
