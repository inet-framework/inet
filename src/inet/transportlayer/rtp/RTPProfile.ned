package inet.transportlayer.rtp;

//
// Generic RTP Profile. ~RTPProfile is internal to the ~RTP module;
// instances are created and connected dynamically by ~RTP.
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
// Audio-Video RTP Profile. ~RTPAVProfile is internal to the ~RTP module;
// instances are created and connected dynamically by ~RTP.
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
