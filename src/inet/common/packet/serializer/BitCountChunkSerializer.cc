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
#include "inet/common/packet/serializer/SerializerRegistry.h"

namespace inet {

Register_Serializer(BitCountChunk, BitCountChunkSerializer);

void BitCountChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& bitCountChunk = std::static_pointer_cast<const BitCountChunk>(chunk);
    bit serializedLength = length == bit(-1) ? bitCountChunk->getChunkLength() - offset : length;
    stream.writeBitRepeatedly(false, bit(serializedLength).get());
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BitCountChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitCountChunk = std::make_shared<BitCountChunk>();
    bit length = byte(stream.getRemainingSize());
    stream.readBitRepeatedly(false, bit(length).get());
    bitCountChunk->setLength(bit(length));
    ChunkSerializer::totalDeserializedBitCount += length;
    return bitCountChunk;
}

} // namespace
