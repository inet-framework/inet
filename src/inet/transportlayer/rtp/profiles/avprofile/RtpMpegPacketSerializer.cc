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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpMpegPacket_m.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpMpegPacketSerializer.h"

namespace inet::rtp {

Register_Serializer(RtpMpegHeader, RtpMpegPacketSerializer);

void RtpMpegPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& rtpMpegHeader = staticPtrCast<const RtpMpegHeader>(chunk);
    stream.writeUint16Be(rtpMpegHeader->getPayloadLength());
    stream.writeUint16Be(rtpMpegHeader->getPictureType());
}

const Ptr<Chunk> RtpMpegPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto rtpMpegHeader = makeShared<RtpMpegHeader>();
    rtpMpegHeader->setPayloadLength(stream.readUint16Be());
    rtpMpegHeader->setPictureType(stream.readUint16Be());
    return rtpMpegHeader;
}

} // namespace inet::rtp


















