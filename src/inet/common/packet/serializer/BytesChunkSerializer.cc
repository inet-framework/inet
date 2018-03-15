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

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/serializer/BytesChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BytesChunk, BytesChunkSerializer);

void BytesChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& bytesChunk = staticPtrCast<const BytesChunk>(chunk);
    b serializedLength = length == b(-1) ? bytesChunk->getChunkLength() - offset: length;
    stream.writeBytes(bytesChunk->getBytes(), offset, serializedLength);
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> BytesChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bytesChunk = makeShared<BytesChunk>();
    B length = stream.getRemainingLength();
    std::vector<uint8_t> chunkBytes;
    stream.readBytes(chunkBytes, length);
    bytesChunk->setBytes(chunkBytes);
    ChunkSerializer::totalDeserializedLength += length;
    return bytesChunk;
}

} // namespace
