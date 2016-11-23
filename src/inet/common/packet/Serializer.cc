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

#include "inet/common/packet/Serializer.h"

namespace inet {

Register_Serializer(BytesChunkSerializer);
Register_Serializer(LengthChunkSerializer);
Register_Serializer(SliceChunkSerializer);
Register_Serializer(SequenceChunkSerializer);

int64_t ChunkSerializer::totalSerializedLength = 0;
int64_t ChunkSerializer::totalDeserializedLength = 0;

void BytesChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& byteArrayChunk = std::static_pointer_cast<const BytesChunk>(chunk);
    stream.writeBytes(byteArrayChunk->getBytes());
}

std::shared_ptr<Chunk> BytesChunkSerializer::deserialize(ByteInputStream& stream) const
{
    auto byteArrayChunk = std::make_shared<BytesChunk>();
    int byteLength = stream.getRemainingSize();
    std::vector<uint8_t> chunkBytes;
    for (int64_t i = 0; i < byteLength; i++)
        chunkBytes.push_back(stream.readByte());
    byteArrayChunk->setBytes(chunkBytes);
    return byteArrayChunk;
}

void LengthChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& lengthChunk = std::static_pointer_cast<const LengthChunk>(chunk);
    stream.writeByteRepeatedly('?', lengthChunk->getChunkLength());
}

std::shared_ptr<Chunk> LengthChunkSerializer::deserialize(ByteInputStream& stream) const
{
    auto lengthChunk = std::make_shared<LengthChunk>();
    int length = stream.getRemainingSize();
    stream.readByteRepeatedly('?', length);
    lengthChunk->setByteLength(length);
    return lengthChunk;
}

void SliceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& sliceChunk = std::static_pointer_cast<const SliceChunk>(chunk);
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, sliceChunk->getChunk());
    stream.writeBytes(outputStream.getBytes(), sliceChunk->getByteOffset(), sliceChunk->getChunkLength());
}

std::shared_ptr<Chunk> SliceChunkSerializer::deserialize(ByteInputStream& stream) const
{
    throw cRuntimeError("Invalid operation");
}

void SequenceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& sequenceChunk = std::static_pointer_cast<const SequenceChunk>(chunk);
    for (auto& chunk : sequenceChunk->getChunks())
        Chunk::serialize(stream, chunk);
}

std::shared_ptr<Chunk> SequenceChunkSerializer::deserialize(ByteInputStream& stream) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace
