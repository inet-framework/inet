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

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/serializer/ByteCountChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(ByteCountChunk, ByteCountChunkSerializer);

void ByteCountChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& byteCountChunk = std::static_pointer_cast<const ByteCountChunk>(chunk);
    bit serializedLength = length == bit(-1) ? byteCountChunk->getChunkLength() - offset : length;
    stream.writeByteRepeatedly('?', byte(serializedLength).get());
    ChunkSerializer::totalSerializedLength += serializedLength;
}

Ptr<Chunk> ByteCountChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteCountChunk = std::make_shared<ByteCountChunk>();
    byte length = stream.getRemainingLength();
    stream.readByteRepeatedly('?', byte(length).get());
    byteCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedLength += length;
    return byteCountChunk;
}

} // namespace
