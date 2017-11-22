package inet.transportlayer.rtp;

//
// Generic Rtp Profile. ~RtpProfile is internal to the ~Rtp module;
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
// Audio-Video Rtp Profile. ~RtpAvProfile is internal to the ~Rtp module;
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
