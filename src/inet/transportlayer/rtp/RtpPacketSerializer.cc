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
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/RtpPacketSerializer.h"

namespace inet {

namespace rtp {

Register_Serializer(RtpHeader, RtpPacketSerializer);

void RtpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& rtpHeader = staticPtrCast<const RtpHeader>(chunk);
    stream.writeNBitsOfUint64Be(rtpHeader->getVersion(), 2);
    stream.writeBit(rtpHeader->getPaddingFlag());
    stream.writeBit(rtpHeader->getExtensionFlag());
    size_t csrcArraySize = rtpHeader->getCsrcArraySize();
    stream.writeUint4(csrcArraySize);
    stream.writeBit(rtpHeader->getMarker());
    stream.writeNBitsOfUint64Be(rtpHeader->getPayloadType(), 7);
    stream.writeUint16Be(rtpHeader->getSequenceNumber());
    stream.writeUint32Be(rtpHeader->getTimeStamp());
    stream.writeUint32Be(rtpHeader->getSsrc());
    for(size_t i = 0; i < csrcArraySize; ++i){
        stream.writeUint32Be(rtpHeader->getCsrc(i));
    }
}

const Ptr<Chunk> RtpPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto rtpHeader = makeShared<RtpHeader>();
    rtpHeader->setVersion(stream.readNBitsToUint64Be(2));
    rtpHeader->setPaddingFlag(stream.readBit());
    rtpHeader->setExtensionFlag(stream.readBit());
    size_t csrcArraySize = stream.readUint4();
    rtpHeader->setCsrcArraySize(csrcArraySize);
    rtpHeader->setMarker(stream.readBit());
    rtpHeader->setPayloadType(stream.readNBitsToUint64Be(7));
    rtpHeader->setSequenceNumber(stream.readUint16Be());
    rtpHeader->setTimeStamp(stream.readUint32Be());
    rtpHeader->setSsrc(stream.readUint32Be());
    for(size_t i = 0; i < csrcArraySize; ++i){
        rtpHeader->setCsrc(i, stream.readUint32Be());
    }
    rtpHeader->setChunkLength(B(12 + csrcArraySize * 4));
    return rtpHeader;
}

} // namespace rtp

} // namespace inet

