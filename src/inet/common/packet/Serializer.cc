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
#include "inet/common/packet/SerializerRegistry.h"

namespace inet {

Register_Serializer(BitCountChunk, BitCountChunkSerializer);
Register_Serializer(BitsChunk, BitsChunkSerializer);
Register_Serializer(ByteCountChunk, ByteCountChunkSerializer);
Register_Serializer(BytesChunk, BytesChunkSerializer);
Register_Serializer(SequenceChunk, SequenceChunkSerializer);
Register_Serializer(SliceChunk, SliceChunkSerializer);

int64_t ChunkSerializer::totalSerializedBitCount = 0;
int64_t ChunkSerializer::totalDeserializedBitCount = 0;

void BitCountChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& bitCountChunk = std::static_pointer_cast<const BitCountChunk>(chunk);
    int64_t serializedLength = length == -1 ? bitCountChunk->getChunkLength() - offset : length;
    stream.writeBitRepeatedly(false, serializedLength);
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BitCountChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitCountChunk = std::make_shared<BitCountChunk>();
    int64_t length = stream.getRemainingSize() * 8;
    stream.readBitRepeatedly(false, length);
    bitCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedBitCount += length;
    return bitCountChunk;
}

void BitsChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& bitsChunk = std::static_pointer_cast<const BitsChunk>(chunk);
    int64_t serializedLength = length == -1 ? bitsChunk->getChunkLength() - offset: length;
    stream.writeBits(bitsChunk->getBits(), offset, serializedLength);
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BitsChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitsChunk = std::make_shared<BitsChunk>();
    int64_t length = stream.getRemainingSize() * 8;
    std::vector<bool> chunkBits;
    for (int64_t i = 0; i < length; i++)
        chunkBits.push_back(stream.readBit());
    bitsChunk->setBits(chunkBits);
    ChunkSerializer::totalDeserializedBitCount += length;
    return bitsChunk;
}

void ByteCountChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& byteCountChunk = std::static_pointer_cast<const ByteCountChunk>(chunk);
    int64_t serializedLength = length == -1 ? byteCountChunk->getChunkLength() - offset : length;
    assert(serializedLength % 8 == 0);
    stream.writeByteRepeatedly('?', serializedLength / 8);
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> ByteCountChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteCountChunk = std::make_shared<ByteCountChunk>();
    int64_t length = stream.getRemainingSize();
    stream.readByteRepeatedly('?', length);
    byteCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedBitCount += length * 8;
    return byteCountChunk;
}

void BytesChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const
{
    const auto& bytesChunk = std::static_pointer_cast<const BytesChunk>(chunk);
    int64_t serializedLength = length == -1 ? bytesChunk->getChunkLength() - offset: length;
    assert(offset % 8 == 0);
    assert(serializedLength % 8 == 0);
    stream.writeBytes(bytesChunk->getBytes(), offset / 8, serializedLength / 8);
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BytesChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bytesChunk = std::make_shared<BytesChunk>();
    int64_t length = stream.getRemainingSize();
    std::vector<uint8_t> chunkBytes;
    for (int64_t i = 0; i < length; i++)
        chunkBytes.push_back(stream.readByte());
    bytesChunk->setBytes(chunkBytes);
    ChunkSerializer::totalDeserializedBitCount += length * 8;
    return bytesChunk;
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
        stream.writeBytes(*fieldsChunk->getSerializedBytes(), offset / 8, length == -1 ? length : length / 8);
    else if (offset == 0 && (length == -1 || length == chunk->getChunkLength())) {
        auto streamPosition = stream.getPosition();
        serialize(stream, fieldsChunk);
        int64_t serializedLength = stream.getPosition() - streamPosition;
        ChunkSerializer::totalSerializedBitCount += serializedLength * 8;
        fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, serializedLength));
    }
    else {
        ByteOutputStream chunkStream;
        serialize(chunkStream, fieldsChunk);
        stream.writeBytes(chunkStream.getBytes(), offset / 8, length == -1 ? length : length / 8);
        ChunkSerializer::totalSerializedBitCount += chunkStream.getSize();
        fieldsChunk->setSerializedBytes(chunkStream.copyBytes());
    }
}

std::shared_ptr<Chunk> FieldsChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto streamPosition = stream.getPosition();
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(deserialize(stream));
    auto length = stream.getPosition() - streamPosition;
    ChunkSerializer::totalDeserializedBitCount += length * 8;
    fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, length));
    return fieldsChunk;
}

} // namespace
