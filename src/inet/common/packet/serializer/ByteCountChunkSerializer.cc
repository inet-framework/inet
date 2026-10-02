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
    // A slice can start and end inside a byte: every byte holds the same value, so the
    // bit at position p is bit (p % 8) of that value, counted from the most significant.
    uint8_t data = byteCountChunk->getData();
    int64_t position = offset.get<b>();
    int64_t end = position + serializedLength.get<b>();
    for (; position < end && position % 8 != 0; position++)
        stream.writeBit(data & (0x80 >> (position % 8)));
    int64_t byteCount = (end - position) / 8;
    stream.writeByteRepeatedly(data, byteCount);
    position += 8 * byteCount;
    for (; position < end; position++)
        stream.writeBit(data & (0x80 >> (position % 8)));
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

