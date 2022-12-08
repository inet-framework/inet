//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/StreamBufferChunkSerializer.h"

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(StreamBufferChunk, StreamBufferChunkSerializer);

void StreamBufferChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& streamBufferChunk = staticPtrCast<const StreamBufferChunk>(chunk);
    b currentStreamPosition = b((simTime() - streamBufferChunk->getStartTime()).dbl() * streamBufferChunk->getDatarate().get());
    if (offset < currentStreamPosition)
        Chunk::serialize(stream, streamBufferChunk->getStreamData(), offset, std::min(length, currentStreamPosition - offset));
    if (offset + length > currentStreamPosition)
        stream.writeBitRepeatedly(false, b(offset + length - currentStreamPosition).get());
}

const Ptr<Chunk> StreamBufferChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace

