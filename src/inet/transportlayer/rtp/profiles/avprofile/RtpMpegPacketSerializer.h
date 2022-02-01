//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RTPMPEGPACKETSERIALIZER_H
#define __INET_RTPMPEGPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace rtp {

/**
 * Converts between RtpMpegPacket and binary (network byte order) RTP MPEG packet.
 */
class INET_API RtpMpegPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    RtpMpegPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace rtp
} // namespace inet

#endif

