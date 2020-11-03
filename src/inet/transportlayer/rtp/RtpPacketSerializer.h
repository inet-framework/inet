//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

