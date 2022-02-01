//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

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

simple RtpPayloadSender like IRtpPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Sender like IRtpPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Sender like IRtpPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioSender like IRtpPayloadSender
{
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

