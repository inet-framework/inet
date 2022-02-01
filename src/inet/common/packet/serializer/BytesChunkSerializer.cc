//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/BytesChunkSerializer.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BytesChunk, BytesChunkSerializer);

void BytesChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& bytesChunk = staticPtrCast<const BytesChunk>(chunk);
    b serializedLength = length == b(-1) ? bytesChunk->getChunkLength() - offset : length;
    stream.writeBytes(bytesChunk->getBytes(), offset, serializedLength);
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> BytesChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bytesChunk = makeShared<BytesChunk>();
    B length = stream.getRemainingLength();
    std::vector<uint8_t> chunkBytes;
    stream.readBytes(chunkBytes, length);
    bytesChunk->setBytes(chunkBytes);
    ChunkSerializer::totalDeserializedLength += length;
    return bytesChunk;
}

} // namespace

