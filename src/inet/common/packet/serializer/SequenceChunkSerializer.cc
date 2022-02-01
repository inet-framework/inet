//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/SequenceChunkSerializer.h"

#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(SequenceChunk, SequenceChunkSerializer);

void SequenceChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    b currentOffset = b(0);
    b serializeBegin = offset;
    b serializeEnd = length == b(-1) ? chunk->getChunkLength() : offset + length;

    const auto& sequenceChunk = staticPtrCast<const SequenceChunk>(chunk);
    for (auto& chunk : sequenceChunk->getChunks()) {
        b chunkLength = chunk->getChunkLength();
        b chunkBegin = currentOffset;
        b chunkEnd = currentOffset + chunkLength;
        if (serializeBegin <= chunkBegin && chunkEnd <= serializeEnd)
            Chunk::serialize(stream, chunk);
        else if (chunkBegin < serializeBegin && serializeEnd < chunkEnd)
            Chunk::serialize(stream, chunk, serializeBegin - chunkBegin, serializeEnd - serializeBegin);
        else if (chunkBegin < serializeBegin && serializeBegin < chunkEnd)
            Chunk::serialize(stream, chunk, serializeBegin - chunkBegin, chunkEnd - serializeBegin);
        else if (chunkBegin < serializeEnd && serializeEnd < chunkEnd)
            Chunk::serialize(stream, chunk, b(0), serializeEnd - chunkBegin);
        // otherwise the element chunk is out of the slice, therefore it's ignored
        currentOffset += chunkLength;
    }
}

const Ptr<Chunk> SequenceChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace

