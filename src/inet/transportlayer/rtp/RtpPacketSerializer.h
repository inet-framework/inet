//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RTPPACKETSERIALIZER_H
#define __INET_RTPPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace rtp {

/**
 * Converts between RtpPacket and binary (network byte order) RTP packet.
 */
class INET_API RtpPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    RtpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace rtp
} // namespace inet

#endif

