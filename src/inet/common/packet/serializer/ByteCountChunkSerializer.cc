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
    stream.writeByteRepeatedly(byteCountChunk->getData(), B(serializedLength).get());
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> ByteCountChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteCountChunk = makeShared<ByteCountChunk>();
    B length = stream.getRemainingLength();
    stream.readByteRepeatedly(byteCountChunk->getData(), B(length).get());
    byteCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedLength += length;
    return byteCountChunk;
}

} // namespace

