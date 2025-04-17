//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

import inet.common.SimpleModule;

//
// Generic RTP Profile. ~RtpProfile is internal to the ~Rtp module;
// instances are created and connected dynamically by ~Rtp.
//
simple RtpProfile extends SimpleModule
{
    parameters:
        @class(RtpProfile);
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
simple RtpAvProfile extends SimpleModule
{
    parameters:
        @class(RtpAvProfile);
        bool autoOutputFileNames;

    gates:
        input rtpIn;
        input payloadSenderIn;
        input payloadReceiverIn[];
        output rtpOut;
        output payloadSenderOut;
        output payloadReceiverOut[];
}

