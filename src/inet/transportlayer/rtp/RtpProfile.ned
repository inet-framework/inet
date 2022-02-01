//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

//
// Generic RTP Profile. ~RtpProfile is internal to the ~Rtp module;
// instances are created and connected dynamically by ~Rtp.
//
simple RtpProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input rtpIn;
        input payloadSenderIn;
        input payloadReceiverIn[];
        output rtpOut @labels(RtpInnerPacket);
        output payloadSenderOut;
        output payloadReceiverOut[];
}

//
// Audio-Video RTP Profile. ~RtpAvProfile is internal to the ~Rtp module;
// instances are created and connected dynamically by ~Rtp.
//
simple RtpAvProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input rtpIn;
        input payloadSenderIn;
        input payloadReceiverIn[];
        output rtpOut;
        output payloadSenderOut;
        output payloadReceiverOut[];
}

