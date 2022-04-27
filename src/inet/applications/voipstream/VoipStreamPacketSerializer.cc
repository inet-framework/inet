//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/voipstream/VoipStreamPacketSerializer.h"

#include "inet/applications/voipstream/VoipStreamPacket_m.h"
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

