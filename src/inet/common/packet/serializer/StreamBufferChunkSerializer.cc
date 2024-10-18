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
    if (length == b(-1))
        length = streamBufferChunk->getChunkLength() - offset;
    b copyLength = std::max(b(0), std::min(length, currentStreamPosition - offset));
    // KLUDGE: round up to next value divisible by 8, because serialization doesn't work with arbitrary bit lengths
    // this rounding doesn't really affect the outcome of the simulation, the limited serialization length just
    // protects against using data that isn't streamed in the buffer yet
    copyLength = b(b((copyLength.get() + 7) /  8) * 8);
    if (copyLength > b(0))
        Chunk::serialize(stream, streamBufferChunk->getStreamData(), offset, copyLength);
    if (copyLength < length)
        stream.writeBitRepeatedly(false, b(length - copyLength).get());
}

const Ptr<Chunk> StreamBufferChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace

