//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/rtp/RtpPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"

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
    for (size_t i = 0; i < csrcArraySize; ++i) {
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
    for (size_t i = 0; i < csrcArraySize; ++i) {
        rtpHeader->setCsrc(i, stream.readUint32Be());
    }
    rtpHeader->setChunkLength(B(12 + csrcArraySize * 4));
    return rtpHeader;
}

} // namespace rtp

} // namespace inet

