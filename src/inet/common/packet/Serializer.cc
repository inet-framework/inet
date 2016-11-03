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

Register_Serializer(ByteArrayChunkSerializer);
Register_Serializer(ByteLengthChunkSerializer);
Register_Serializer(SliceChunkSerializer);
Register_Serializer(SequenceChunkSerializer);

void ByteArrayChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& byteArrayChunk = std::static_pointer_cast<const ByteArrayChunk>(chunk);
    stream.writeBytes(byteArrayChunk->getBytes());
}

std::shared_ptr<Chunk> ByteArrayChunkSerializer::deserialize(ByteInputStream& stream) const
{
    auto byteArrayChunk = std::make_shared<ByteArrayChunk>();
    int byteLength = stream.getRemainingSize();
    std::vector<uint8_t> chunkBytes;
    for (int64_t i = 0; i < byteLength; i++)
        chunkBytes.push_back(stream.readByte());
    byteArrayChunk->setBytes(chunkBytes);
    return byteArrayChunk;
}

void ByteLengthChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& byteLengthChunk = std::static_pointer_cast<const ByteLengthChunk>(chunk);
    stream.writeByteRepeatedly('?', byteLengthChunk->getByteLength());
}

std::shared_ptr<Chunk> ByteLengthChunkSerializer::deserialize(ByteInputStream& stream) const
{
    auto byteLengthChunk = std::make_shared<ByteLengthChunk>();
    int byteLength = stream.getRemainingSize();
    stream.readByteRepeatedly('?', byteLength);
    byteLengthChunk->setByteLength(byteLength);
    return byteLengthChunk;
}

void SliceChunkSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& sliceChunk = std::static_pointer_cast<const SliceChunk>(chunk);
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, sliceChunk->getChunk());
    stream.writeBytes(outputStream.getBytes(), sliceChunk->getByteOffset(), sliceChunk->getByteLength());
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
