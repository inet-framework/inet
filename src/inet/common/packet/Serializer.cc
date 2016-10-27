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

Register_Class(ByteArrayChunkSerializer);
Register_Class(ByteLengthChunkSerializer);
Register_Class(SliceChunkSerializer);
Register_Class(SequenceChunkSerializer);

void ByteArrayChunkSerializer::deserialize(ByteInputStream& stream, Chunk& chunk)
{
    auto& byteArrayChunk = static_cast<ByteArrayChunk&>(chunk);
    int byteLength = stream.getRemainingSize();
    std::vector<uint8_t> chunkBytes;
    for (int64_t i = 0; i < byteLength; i++)
        chunkBytes.push_back(stream.readByte());
    byteArrayChunk.setBytes(chunkBytes);
}

void ByteArrayChunkSerializer::serialize(ByteOutputStream& stream, const Chunk& chunk) const
{
    auto& byteArrayChunk = static_cast<const ByteArrayChunk&>(chunk);
    stream.writeBytes(byteArrayChunk.getBytes());
}

void ByteLengthChunkSerializer::serialize(ByteOutputStream& stream, const Chunk& chunk) const
{
    auto& byteLengthChunk = static_cast<const ByteLengthChunk&>(chunk);
    stream.writeByteRepeatedly('?', byteLengthChunk.getByteLength());
}

void ByteLengthChunkSerializer::deserialize(ByteInputStream& stream, Chunk& chunk)
{
    auto& byteLengthChunk = static_cast<ByteLengthChunk&>(chunk);
    int byteLength = stream.getRemainingSize();
    stream.readByteRepeatedly('?', byteLength);
    byteLengthChunk.setByteLength(byteLength);
}

void SliceChunkSerializer::serialize(ByteOutputStream& stream, const Chunk& chunk) const
{
    auto& sliceChunk = static_cast<const SliceChunk&>(chunk);
    ByteOutputStream outputStream;
    sliceChunk.getChunk()->serialize(outputStream);
    stream.writeBytes(outputStream.getBytes(), sliceChunk.getByteOffset(), sliceChunk.getByteLength());
}

void SliceChunkSerializer::deserialize(ByteInputStream& stream, Chunk& chunk)
{
    throw cRuntimeError("Invalid operation");
}

void SequenceChunkSerializer::serialize(ByteOutputStream& stream, const Chunk& chunk) const
{
    auto& sequenceChunk = static_cast<const SequenceChunk&>(chunk);
    for (auto& chunk : sequenceChunk.getChunks())
        chunk->serialize(stream);
}

void SequenceChunkSerializer::deserialize(ByteInputStream& stream, Chunk& chunk)
{
    throw cRuntimeError("Invalid operation");
}
