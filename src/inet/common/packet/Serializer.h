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

#ifndef __INET_SERIALIZER_H_
#define __INET_SERIALIZER_H_

#include "inet/common/packet/ByteStream.h"
#include "inet/common/packet/ByteArrayChunk.h"
#include "inet/common/packet/ByteLengthChunk.h"
#include "inet/common/packet/SliceChunk.h"
#include "inet/common/packet/SequenceChunk.h"

class ChunkSerializer : public cObject
{
  public:
    virtual ~ChunkSerializer() { }

    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const = 0;
    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) = 0;
};

class ByteArrayChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        auto& byteArrayChunk = static_cast<const ByteArrayChunk&>(chunk);
        stream.writeBytes(byteArrayChunk.getBytes());
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        auto& byteArrayChunk = static_cast<ByteArrayChunk&>(chunk);
        int byteLength = stream.getRemainingSize();
        std::vector<uint8_t> chunkBytes;
        for (int64_t i = 0; i < byteLength; i++)
            chunkBytes.push_back(stream.readByte());
        byteArrayChunk.setBytes(chunkBytes);
    }
};

class ByteLengthChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        auto& byteLengthChunk = static_cast<const ByteLengthChunk&>(chunk);
        stream.writeByteRepeatedly('?', byteLengthChunk.getByteLength());
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        auto& byteLengthChunk = static_cast<ByteLengthChunk&>(chunk);
        int byteLength = stream.getRemainingSize();
        stream.readByteRepeatedly('?', byteLength);
        byteLengthChunk.setByteLength(byteLength);
    }
};

class SliceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        auto& sliceChunk = static_cast<const SliceChunk&>(chunk);
        ByteOutputStream outputStream;
        sliceChunk.getChunk()->serialize(outputStream);
        stream.writeBytes(outputStream.getBytes(), sliceChunk.getByteOffset(), sliceChunk.getByteLength());
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        throw cRuntimeError("Invalid operation");
    }
};

class SequenceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        auto& sequenceChunk = static_cast<const SequenceChunk&>(chunk);
        for (auto& chunk : sequenceChunk.getChunks())
            chunk->serialize(stream);
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        throw cRuntimeError("Invalid operation");
    }
};

#endif // #ifndef __INET_SERIALIZER_H_

