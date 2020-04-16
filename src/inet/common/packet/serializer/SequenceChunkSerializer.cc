//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/serializer/SequenceChunkSerializer.h"
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
