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

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/serializer/BitsChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BitsChunk, BitsChunkSerializer);

void BitsChunkSerializer::serialize(MemoryOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& bitsChunk = std::static_pointer_cast<const BitsChunk>(chunk);
    bit serializedLength = length == bit(-1) ? bitsChunk->getChunkLength() - offset: length;
    stream.writeBits(bitsChunk->getBits(), offset, serializedLength);
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BitsChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitsChunk = std::make_shared<BitsChunk>();
    bit length = byte(stream.getRemainingSize());
    std::vector<bool> chunkBits;
    stream.readBits(chunkBits, 0, bit(length).get());
    bitsChunk->setBits(chunkBits);
    ChunkSerializer::totalDeserializedBitCount += length;
    return bitsChunk;
}

} // namespace
