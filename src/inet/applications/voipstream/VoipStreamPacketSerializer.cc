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

#include "inet/applications/voipstream/VoipStreamPacket_m.h"
#include "inet/applications/voipstream/VoipStreamPacketSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(VoipStreamPacket, VoipStreamPacketSerializer);

void VoipStreamPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& voipStreamPacket = staticPtrCast<const VoipStreamPacket>(chunk);
    stream.writeByte(voipStreamPacket->getHeaderLength());
    VoipStreamPacketType type = voipStreamPacket->getType();
    stream.writeByte(type);
    stream.writeUint32Be(voipStreamPacket->getCodec());
    stream.writeUint16Be(voipStreamPacket->getSampleBits());
    stream.writeUint16Be(voipStreamPacket->getSampleRate());
    stream.writeUint32Be(voipStreamPacket->getTransmitBitrate());
    stream.writeUint16Be(voipStreamPacket->getSamplesPerPacket());
    stream.writeUint16Be(voipStreamPacket->getSeqNo());
    stream.writeUint32Be(voipStreamPacket->getTimeStamp());
    stream.writeUint32Be(voipStreamPacket->getSsrc());
    if (type == VOICE)
        stream.writeUint16Be(voipStreamPacket->getDataLength());

    int64_t remainders = B(voipStreamPacket->getHeaderLength()).get() - B((stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("voipStreamPacket length = %d smaller than required %d bytes, try to increment the 'voipHeaderSize' parameter", (int)B(voipStreamPacket->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> VoipStreamPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto voipStreamPacket = makeShared<VoipStreamPacket>();
    int headerLength = stream.readByte();
    voipStreamPacket->setHeaderLength(headerLength);
    VoipStreamPacketType type = static_cast<VoipStreamPacketType>(stream.readByte());
    voipStreamPacket->setType(type);
    voipStreamPacket->setCodec(stream.readUint32Be());
    voipStreamPacket->setSampleBits(stream.readUint16Be());
    voipStreamPacket->setSampleRate(stream.readUint16Be());
    voipStreamPacket->setTransmitBitrate(stream.readUint32Be());
    voipStreamPacket->setSamplesPerPacket(stream.readUint16Be());
    voipStreamPacket->setSeqNo(stream.readUint16Be());
    voipStreamPacket->setTimeStamp(stream.readUint32Be());
    voipStreamPacket->setSsrc(stream.readUint32Be());
    if (type == VOICE)
        voipStreamPacket->setDataLength(stream.readUint16Be());

    B remainders = B(headerLength) - (stream.getPosition() - startPosition);
    if (remainders.get() < 0) {
        voipStreamPacket->markIncorrect();
        return voipStreamPacket;
    }
    stream.readByteRepeatedly('?', B(remainders).get());
    return voipStreamPacket;
}

} // namespace inet

