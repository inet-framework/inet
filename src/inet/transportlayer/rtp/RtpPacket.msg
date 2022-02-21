//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::rtp;

cplusplus {{
    const B RTPPACKET_FIX_HEADERLENGTH = B(12);
}}

//
// This class represents an RTP data packet. Real data can either
// be encapsulated, or simulated by adding length.
//
// The following RTP header fields exist but aren't used:
// padding, extension, csrcCount. The csrcList can't be used
// because csrcCount is always 0.
//
class RtpHeader extends FieldsChunk
{
    chunkLength = RTPPACKET_FIX_HEADERLENGTH;  // 12-byte fixed header

    // The rtp version of this ~RtpPacket.
    uint8_t version = 2;      // 2 bits

    // Set to 1 if padding is used in this ~RtpPacket, 0 otherwise.
    // This implementation doesn't use padding bytes, so it is always 0.
    bool paddingFlag = false;      // 1 bit

    // Set to 1, if this ~RtpPacket contains an rtp header extension, 0 otherwise.
    // This implementation doesn't support rtp header extensions, so it is always 0.
    bool extensionFlag = false;      // 1 bit

    // uint8_t cc;      // 4 bits, csrc count

    // The marker.
    bool marker = false;      // 1 bit

    // The type of payload carried in this ~RtpPacket.
    int8_t payloadType;      // 7 bits

    // The sequence number of this ~RtpPacket.
    uint16_t sequenceNumber;      // 16 bits

    // The rtp time stamp of this ~RtpPacket.
    uint32_t timeStamp;      // 32 bits

    // The ssrc identifier of the creator of this ~RtpPacket.
    uint32_t ssrc;      // 32 bits

    // no mixers, no contributing sources
    uint32_t csrc[];      // cc * 32 bits
};

cplusplus(RtpHeader) {{
  public:
    /**
     * Writes a one line info about this RtpHeader into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer description about this RtpHeader into the given stream.
     */
    virtual void dump() const;
}}
