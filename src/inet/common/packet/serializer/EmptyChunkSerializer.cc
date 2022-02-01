//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/EmptyChunkSerializer.h"

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(EmptyChunk, EmptyChunkSerializer);

void EmptyChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    ASSERT(offset == b(0));
    ASSERT(length == b(-1) || length == b(0));
}

const Ptr<Chunk> EmptyChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace

