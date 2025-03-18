//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.transportlayer.rtp;

import inet.common.SimpleModule;

//
// Interface for RTP Payload Senders.
//
// These modules are internal to the ~Rtp module; instances are created
// and connected dynamically by ~RtpProfile.
//
moduleinterface IRtpPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

//
// Base module for RTP payload senders that create RTP data packets. Provides
// common functionality such as opening and closing data files, managing sequence
// numbers and timestamps, and controlling transmission states (playing, stopped).
// Serves as the foundation for specific payload type implementations.
//
simple RtpPayloadSender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RtpPayloadSender);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

//
// Specialized RTP payload sender for PCM audio data (payload type 10) in the
// RTP audio/video profile. Extends the sample-based audio sender to handle
// specific parameters for PCM audio transmission.
//
simple RTPAVProfilePayload10Sender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RTPAVProfilePayload10Sender);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

//
// Specialized RTP payload sender for MPEG video data (payload type 32) in the
// RTP audio/video profile. Reads MPEG frame data from a GDF file and creates
// RTP packets containing MPEG video information with appropriate timing.
//
simple RtpAvProfilePayload32Sender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RtpAvProfilePayload32Sender);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

//
// Base class for RTP payload senders that handle sample-based audio formats in
// the RTP audio/video profile. Provides functionality for reading audio files,
// managing audio parameters (sampling rate, sample width, channels), and
// creating appropriately timed RTP packets containing audio samples.
//
simple RTPAVProfileSampleBasedAudioSender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RTPAVProfileSampleBasedAudioSender);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}
