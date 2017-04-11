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

#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

void FieldsChunkSerializer::serialize(MemoryOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(chunk);
    if (fieldsChunk->getSerializedBytes() != nullptr)
        stream.writeBytes(*fieldsChunk->getSerializedBytes(), offset, length == bit(-1) ? byte(-1) : byte(length));
    else if (offset == bit(0) && (length == bit(-1) || length == chunk->getChunkLength())) {
        auto startPosition = stream.getLength();
        serialize(stream, fieldsChunk);
        bit serializedLength = stream.getLength() - startPosition;
        ChunkSerializer::totalSerializedBitCount += serializedLength;
        auto serializedBytes = new std::vector<uint8_t>();
        stream.copyData(*serializedBytes, startPosition, serializedLength);
        fieldsChunk->setSerializedBytes(serializedBytes);
    }
    else {
        MemoryOutputStream chunkStream(fieldsChunk->getChunkLength());
        serialize(chunkStream, fieldsChunk);
        stream.writeBytes(chunkStream.getData(), offset, length == bit(-1) ? byte(-1) : byte(length));
        ChunkSerializer::totalSerializedBitCount += chunkStream.getLength();
        auto serializedBytes = new std::vector<uint8_t>();
        chunkStream.copyData(*serializedBytes);
        fieldsChunk->setSerializedBytes(serializedBytes);
    }
}

std::shared_ptr<Chunk> FieldsChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(deserialize(stream));
    auto chunkLength = byte(stream.getPosition() - startPosition);
    ChunkSerializer::totalDeserializedBitCount += chunkLength;
    fieldsChunk->setChunkLength(chunkLength);
    auto serializedBytes = new std::vector<uint8_t>();
    stream.copyBytes(*serializedBytes, byte(startPosition).get(), byte(chunkLength).get());
    fieldsChunk->setSerializedBytes(serializedBytes);
    return fieldsChunk;
}

} // namespace
