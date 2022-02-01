//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RTCPPACKETSERIALIZER_H
#define __INET_RTCPPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace rtp {

/**
 * Converts between RtcpPacket and binary (network byte order) RTCP packet.
 */
class INET_API RtcpPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    RtcpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace rtp
} // namespace inet

#endif

