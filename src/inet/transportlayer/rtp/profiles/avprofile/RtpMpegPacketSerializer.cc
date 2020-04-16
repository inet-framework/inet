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

namespace inet {
namespace rtp {

Register_Serializer(RtpMpegHeader, RtpMpegPacketSerializer);

void RtpMpegPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& rtpMpegHeader = staticPtrCast<const RtpMpegHeader>(chunk);
    /*stream.writeNBitsOfUint64Be(rtpMpegHeader->getMbz(), 5);
    stream.writeBit(rtpMpegHeader->getTwo());
    stream.writeNBitsOfUint64Be(rtpMpegHeader->getTemporalReference(), 10);
    stream.writeBit(rtpMpegHeader->getActiveN());
    stream.writeBit(rtpMpegHeader->getNewPictureHeader());
    stream.writeBit(rtpMpegHeader->getSequenceHeaderPresent());
    stream.writeBit(rtpMpegHeader->getBeginningOfSlice());
    stream.writeBit(rtpMpegHeader->getEndOfSlice());
    stream.writeNBitsOfUint64Be(rtpMpegHeader->getPictureType(), 3);
    stream.writeBit(rtpMpegHeader->getFbv());
    stream.writeNBitsOfUint64Be(rtpMpegHeader->getBfc(), 3);
    stream.writeBit(rtpMpegHeader->getFfv());
    stream.writeNBitsOfUint64Be(rtpMpegHeader->getFfc(), 3);*/
    stream.writeUint16Be(rtpMpegHeader->getPayloadLength());
    stream.writeUint16Be(rtpMpegHeader->getPictureType());
}

const Ptr<Chunk> RtpMpegPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto rtpMpegHeader = makeShared<RtpMpegHeader>();
    /*rtpMpegHeader->setMbz(stream.readNBitsToUint64Be(5));
    rtpMpegHeader->setTwo(stream.readBit());
    rtpMpegHeader->setTemporalReference(stream.readNBitsToUint64Be(10));
    rtpMpegHeader->setActiveN(stream.readBit());
    rtpMpegHeader->setNewPictureHeader(stream.readBit());
    rtpMpegHeader->setSequenceHeaderPresent(stream.readBit());
    rtpMpegHeader->setBeginningOfSlice(stream.readBit());
    rtpMpegHeader->setEndOfSlice(stream.readBit());
    rtpMpegHeader->setPictureType(stream.readNBitsToUint64Be(3));
    rtpMpegHeader->setFbv(stream.readBit());
    rtpMpegHeader->setBfc(stream.readNBitsToUint64Be(3));
    rtpMpegHeader->setFfv(stream.readBit());
    rtpMpegHeader->setFfc(stream.readNBitsToUint64Be(3));*/
    rtpMpegHeader->setPayloadLength(stream.readUint16Be());
    rtpMpegHeader->setPictureType(stream.readUint16Be());
    return rtpMpegHeader;
}

} // namespace rtp
} // namespace inet

