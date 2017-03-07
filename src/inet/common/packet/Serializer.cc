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
#include "inet/common/packet/Serializer.h"
#include "inet/common/packet/SerializerRegistry.h"

namespace inet {

Register_Serializer(BitCountChunk, BitCountChunkSerializer);
Register_Serializer(BitsChunk, BitsChunkSerializer);
Register_Serializer(ByteCountChunk, ByteCountChunkSerializer);
Register_Serializer(BytesChunk, BytesChunkSerializer);
Register_Serializer(SequenceChunk, SequenceChunkSerializer);
Register_Serializer(SliceChunk, SliceChunkSerializer);

bit ChunkSerializer::totalSerializedBitCount = bit(0);
bit ChunkSerializer::totalDeserializedBitCount = bit(0);

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

void BitsChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& bitsChunk = std::static_pointer_cast<const BitsChunk>(chunk);
    bit serializedLength = length == bit(-1) ? bitsChunk->getChunkLength() - offset: length;
    stream.writeBits(bitsChunk->getBits(), bit(offset).get(), bit(serializedLength).get());
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BitsChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitsChunk = std::make_shared<BitsChunk>();
    byte length = byte(stream.getRemainingSize());
    std::vector<bool> chunkBits;
    for (byte i = byte(0); i < length; i++)
        chunkBits.push_back(stream.readBit());
    bitsChunk->setBits(chunkBits);
    ChunkSerializer::totalDeserializedBitCount += length;
    return bitsChunk;
}

void ByteCountChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& byteCountChunk = std::static_pointer_cast<const ByteCountChunk>(chunk);
    bit serializedLength = length == bit(-1) ? byteCountChunk->getChunkLength() - offset : length;
    stream.writeByteRepeatedly('?', byte(serializedLength).get());
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> ByteCountChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto byteCountChunk = std::make_shared<ByteCountChunk>();
    byte length = byte(stream.getRemainingSize());
    stream.readByteRepeatedly('?', byte(length).get());
    byteCountChunk->setLength(length);
    ChunkSerializer::totalDeserializedBitCount += length;
    return byteCountChunk;
}

void BytesChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& bytesChunk = std::static_pointer_cast<const BytesChunk>(chunk);
    bit serializedLength = length == bit(-1) ? bytesChunk->getChunkLength() - offset: length;
    stream.writeBytes(bytesChunk->getBytes(), byte(offset).get(), byte(serializedLength).get());
    ChunkSerializer::totalSerializedBitCount += serializedLength;
}

std::shared_ptr<Chunk> BytesChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto bytesChunk = std::make_shared<BytesChunk>();
    byte length = byte(stream.getRemainingSize());
    std::vector<uint8_t> chunkBytes;
    for (byte i = byte(0); i < length; i++)
        chunkBytes.push_back(stream.readByte());
    bytesChunk->setBytes(chunkBytes);
    ChunkSerializer::totalDeserializedBitCount += length;
    return bytesChunk;
}

void SliceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    const auto& sliceChunk = std::static_pointer_cast<const SliceChunk>(chunk);
    Chunk::serialize(stream, sliceChunk->getChunk(), sliceChunk->getOffset() + offset, length == bit(-1) ? sliceChunk->getLength() - offset : length);
}

std::shared_ptr<Chunk> SliceChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

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

void FieldsChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const
{
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(chunk);
    if (fieldsChunk->getSerializedBytes() != nullptr)
        stream.writeBytes(*fieldsChunk->getSerializedBytes(), byte(offset).get(), length == bit(-1) ? -1 : byte(length).get());
    else if (offset == bit(0) && (length == bit(-1) || length == chunk->getChunkLength())) {
        auto streamPosition = stream.getPosition();
        serialize(stream, fieldsChunk);
        bit serializedLength = byte(stream.getPosition() - streamPosition);
        ChunkSerializer::totalSerializedBitCount += serializedLength;
        fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, byte(serializedLength).get()));
    }
    else {
        ByteOutputStream chunkStream;
        serialize(chunkStream, fieldsChunk);
        stream.writeBytes(chunkStream.getBytes(), byte(offset).get(), length == bit(-1) ? -1 : byte(length).get());
        ChunkSerializer::totalSerializedBitCount += byte(chunkStream.getSize());
        fieldsChunk->setSerializedBytes(chunkStream.copyBytes());
    }
}

std::shared_ptr<Chunk> FieldsChunkSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto streamPosition = stream.getPosition();
    auto fieldsChunk = std::static_pointer_cast<FieldsChunk>(deserialize(stream));
    auto length = byte(stream.getPosition() - streamPosition);
    ChunkSerializer::totalDeserializedBitCount += length;
    fieldsChunk->setSerializedBytes(stream.copyBytes(streamPosition, byte(length).get()));
    return fieldsChunk;
}

} // namespace
