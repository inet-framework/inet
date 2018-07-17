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
    const auto& voipStreamPacket = staticPtrCast<const VoipStreamPacket>(chunk);
    stream.writeByte(voipStreamPacket->getType());
    stream.writeUint32Be(voipStreamPacket->getCodec());
    stream.writeUint16Be(voipStreamPacket->getSampleBits());
    stream.writeUint16Be(voipStreamPacket->getSampleRate());
    stream.writeUint16Be(voipStreamPacket->getTransmitBitrate());
    stream.writeUint16Be(voipStreamPacket->getSamplesPerPacket());
    const auto& bytes = voipStreamPacket->getBytes();
    stream.writeUint16Be(bytes.getDataArraySize());
    stream.writeBytes((uint8_t *)bytes.getDataPtr(), B(bytes.getDataArraySize()));

    stream.writeUint16Be(voipStreamPacket->getSeqNo());
    stream.writeUint32Be(voipStreamPacket->getTimeStamp());
    stream.writeUint32Be(voipStreamPacket->getSsrc());
}

const Ptr<Chunk> VoipStreamPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto voipStreamPacket = makeShared<VoipStreamPacket>();
    voipStreamPacket->setType((VoipStreamPacketType)stream.readByte());
    voipStreamPacket->setCodec(stream.readUint32Be());
    voipStreamPacket->setSampleBits(stream.readUint16Be());
    voipStreamPacket->setSampleRate(stream.readUint16Be());
    voipStreamPacket->setTransmitBitrate(stream.readUint16Be());
    voipStreamPacket->setSamplesPerPacket(stream.readUint16Be());
    auto size = stream.readUint16Be();
    auto& bytes = voipStreamPacket->getBytesForUpdate();
    bytes.setDataArraySize(size);
    stream.readBytes((uint8_t *)bytes.getDataPtr(), B(size));
    voipStreamPacket->setSeqNo(stream.readUint16Be());
    voipStreamPacket->setTimeStamp(stream.readUint32Be());
    voipStreamPacket->setSsrc(stream.readUint32Be());
    return voipStreamPacket;
}

} // namespace inet

