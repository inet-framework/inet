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
#include "inet/common/packet/serializer/SerializerRegistry.h"

namespace inet {

Register_Serializer(SequenceChunk, SequenceChunkSerializer);

void SequenceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    bit currentOffset = bit(0);
    bit serializeBegin = offset;
    bit serializeEnd = offset + length == bit(-1) ? chunk->getChunkLength() : length;
    const auto& sequenceChunk = std::static_pointer_cast<const SequenceChunk>(chunk);
    for (auto& chunk : sequenceChunk->getChunks()) {
        bit chunkLength = chunk->getChunkLength();
        bit chunkBegin = currentOffset;
        bit chunkEnd = currentOffset + chunkLength;
        if (serializeBegin <= chunkBegin && chunkEnd <= serializeEnd)
            Chunk::serialize(stream, chunk);
        else if (chunkBegin < serializeBegin && serializeBegin < chunkEnd)
            Chunk::serialize(stream, chunk, serializeBegin - chunkBegin, chunkEnd - serializeBegin);
        else if (chunkBegin < serializeEnd && serializeEnd < chunkEnd)
            Chunk::serialize(stream, chunk, bit(0), chunkEnd - serializeEnd);
        currentOffset += chunkLength;
    }
}

std::shared_ptr<Chunk> SequenceChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace
