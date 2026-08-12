//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/ByteCountChunkSerializer.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(ByteCountChunk, ByteCountChunkSerializer);

void ByteCountChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& byteCountChunk = staticPtrCast<const ByteCountChunk>(chunk);
    b serializedLength = length == b(-1) ? byteCountChunk->getChunkLength() - offset : length;
    stream.writeByteRepeatedly(byteCountChunk->getData(), serializedLength.get<B>());
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> ByteCountChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteCountChunk = makeShared<ByteCountChunk>();
    B length = stream.getRemainingLength();
    if (length > B(0)) {
        // recover the fill value actually on the wire from its first byte, then verify
        // the rest of the stream repeats it
        uint8_t fillValue = stream.readByte();
        byteCountChunk->setData(fillValue);
        if (!stream.readByteRepeatedly(fillValue, (length - B(1)).get<B>()))
            byteCountChunk->markIncorrect();
    }
    byteCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedLength += length;
    return byteCountChunk;
}

} // namespace

