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

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/serializer/BitCountChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BitCountChunk, BitCountChunkSerializer);

void BitCountChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& bitCountChunk = staticPtrCast<const BitCountChunk>(chunk);
    b serializedLength = length == b(-1) ? bitCountChunk->getChunkLength() - offset : length;
    stream.writeBitRepeatedly(bitCountChunk->getData(), b(serializedLength).get());
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> BitCountChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitCountChunk = makeShared<BitCountChunk>();
    b length = stream.getRemainingLength();
    stream.readBitRepeatedly(bitCountChunk->getData(), b(length).get());
    bitCountChunk->setLength(b(length));
    ChunkSerializer::totalDeserializedLength += length;
    return bitCountChunk;
}

} // namespace
