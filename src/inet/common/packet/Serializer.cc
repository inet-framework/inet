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

#include "inet/common/packet/FieldsChunk.h"
#include "inet/common/packet/Serializer.h"

namespace inet {

Register_Serializer(BytesChunkSerializer);
Register_Serializer(LengthChunkSerializer);
Register_Serializer(SliceChunkSerializer);
Register_Serializer(SequenceChunkSerializer);

int64_t ChunkSerializer::totalSerializedLength = 0;
int64_t ChunkSerializer::totalDeserializedLength = 0;

void BytesChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& byteArrayChunk = std::static_pointer_cast<const BytesChunk>(chunk);
    stream.writeBytes(byteArrayChunk->getBytes(), offset, length);
}

std::shared_ptr<Chunk> BytesChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteArrayChunk = std::make_shared<BytesChunk>();
    int length = stream.getRemainingSize();
    std::vector<uint8_t> chunkBytes;
    for (int64_t i = 0; i < length; i++)
        chunkBytes.push_back(stream.readByte());
    byteArrayChunk->setBytes(chunkBytes);
    return byteArrayChunk;
}

void LengthChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& lengthChunk = std::static_pointer_cast<const LengthChunk>(chunk);
    stream.writeByteRepeatedly('?', length == -1 ? lengthChunk->getChunkLength() - offset: length);
}

std::shared_ptr<Chunk> LengthChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto lengthChunk = std::make_shared<LengthChunk>();
    int length = stream.getRemainingSize();
    stream.readByteRepeatedly('?', length);
    lengthChunk->setLength(length);
    return lengthChunk;
}

void SliceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& sliceChunk = std::static_pointer_cast<const SliceChunk>(chunk);
    Chunk::serialize(stream, sliceChunk->getChunk(), sliceChunk->getOffset() + offset, length == -1 ? sliceChunk->getLength() - offset : length);
}

std::shared_ptr<Chunk> SliceChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

void SequenceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    int64_t currentOffset = 0;
    int64_t serializeBegin = offset;
    int64_t serializeEnd = offset + length == -1 ? chunk->getChunkLength() : length;
    const auto& sequenceChunk = std::static_pointer_cast<const SequenceChunk>(chunk);
    for (auto& chunk : sequenceChunk->getChunks()) {
        int64_t chunkLength = chunk->getChunkLength();
        int64_t chunkBegin = currentOffset;
        int64_t chunkEnd = currentOffset + chunkLength;
        if (serializeBegin <= chunkBegin && chunkEnd <= serializeEnd)
            Chunk::serialize(stream, chunk);
        else if (chunkBegin < serializeBegin && serializeBegin < chunkEnd)
            Chunk::serialize(stream, chunk, serializeBegin - chunkBegin, chunkEnd - serializeBegin);
        else if (chunkBegin < serializeEnd && serializeEnd < chunkEnd)
            Chunk::serialize(stream, chunk, 0, chunkEnd - serializeEnd);
        currentOffset += chunkLength;
    }
}

std::shared_ptr<Chunk> SequenceChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

void FieldsChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(chunk);
    if (fieldsChunk->getSerializedBytes() != nullptr)
        stream.writeBytes(*fieldsChunk->getSerializedBytes(), offset, length);
    else if (offset == 0 && (length == -1 || length == chunk->getChunkLength())) {
        auto streamPosition = stream.getPosition();
        serialize(stream, fieldsChunk);
        int64_t serializedLength = stream.getPosition() - streamPosition;
        ChunkSerializer::totalSerializedLength += serializedLength;
        fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, serializedLength));
    }
    else {
        ByteOutputStream chunkStream;
        serialize(chunkStream, fieldsChunk);
        stream.writeBytes(chunkStream.getBytes(), offset, length);
        ChunkSerializer::totalSerializedLength += chunkStream.getSize();
        fieldsChunk->setSerializedBytes(chunkStream.copyBytes());
    }
}

std::shared_ptr<Chunk> FieldsChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto streamPosition = stream.getPosition();
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(deserialize(stream));
    auto length = stream.getPosition() - streamPosition;
    ChunkSerializer::totalDeserializedLength += length;
    fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, length));
    return fieldsChunk;
}

} // namespace
