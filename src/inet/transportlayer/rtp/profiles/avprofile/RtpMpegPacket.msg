//
// Copyright (C) 2008 OpenSim Ltd.
// Copyright (C) 2001 Matthias Oppitz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// An ~RtpMpegPacket is intended to be capsulated into an ~RtpPacket when
// transmitting mpeg data with rtp under the rtp audio/video profile.
// It stores information about the mpeg data as described in rfc 2250.
// This implementation doesn't transport real mpeg data. It is intended
// to simulate storing mpeg data by adding length.
// Currently only one header field (picture type) is filled with the right
// value. The values for the other header fields can't be determined by
// reading the gdf file.
//
// @see ~RtpAvProfilePayload32Sender
// @see ~RtpAvProfilePayload32Receiver
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::rtp;

class RtpMpegHeader extends FieldsChunk
{
    chunkLength = B(4);
    int headerLength = 4;
    int payloadLength; // size of the mpeg data
    uint8_t mbz = 0; // Unused. Must be set to zero in current specification.
    bool two = false; // Set to 1 when the MPEG-2 video-specific header extension follows this header.
    uint16_t temporalReference = 0; // The temporal reference of the current picture within the current GOP.
    bool activeN = false; // Set to 1 when the following bit (N) is used to signal changes in the picture header information for MPEG-2 payloads.
    bool newPictureHeader = false; // Used for MPEG-2 payloads when the previous bit (AN) is set to 1. Otherwise, it must be set to zero.
    bool sequenceHeaderPresent = false; // Normally 0 and set to 1 at the occurrence of each MPEG sequence header.
    bool beginningOfSlice = false; // Set when the start of the packet payload is a slice start code, or when a slice start
                                   // code is preceded only by one or more of a Video_Sequence_Header, GOP_header and/or Picture_Header.
    bool endOfSlice = false; // Set when the last byte of the payload is the end of an MPEG slice.
    uint8_t pictureType; // picture type of the frame the data in this packet belongs to.
    bool fbv = false; // full_pel_backward_vector
    uint8_t bfc = 0; // backward_f_code
    bool ffv = false; // full_pel_forward_vector
    uint8_t ffc = 0; // forward_f_code

    // currenrtly unused fields:
    //int mzb;
    //int two;
    //int temporalReference;
    //int activeN;
    //int newPictureHeader;
    //int sequenceHeaderPresent;
    //int beginningOfSlice;
    //int endOfSlice;
    //int fbv;
    //int bfc;
    //int ffv;
    //int ffc;
}
