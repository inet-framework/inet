//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


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

simple RtpPayloadSender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RtpPayloadSender);
        
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Sender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RTPAVProfilePayload10Sender);
        
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Sender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RtpAvProfilePayload32Sender);
        
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioSender extends SimpleModule like IRtpPayloadSender
{
    parameters:
        @class(RTPAVProfileSampleBasedAudioSender);
        
    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

