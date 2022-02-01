//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/SliceChunkSerializer.h"

#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(SliceChunk, SliceChunkSerializer);

void SliceChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& sliceChunk = staticPtrCast<const SliceChunk>(chunk);
    Chunk::serialize(stream, sliceChunk->getChunk(), sliceChunk->getOffset() + offset, length == b(-1) ? sliceChunk->getLength() - offset : length);
}

const Ptr<Chunk> SliceChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace

