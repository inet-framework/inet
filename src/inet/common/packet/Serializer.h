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

namespace inet {

#define Register_Serializer(CLASSNAME) Register_Class(CLASSNAME)

class ChunkSerializer : public cObject
{
  public:
    static int64_t totalSerializedByteLength;
    static int64_t totalDeserializedByteLength;

  public:
    virtual ~ChunkSerializer() { }

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const = 0;
};

class ByteArrayChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

class ByteLengthChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

class SliceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

class SequenceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const;
};

} // namespace

#endif // #ifndef __INET_SERIALIZER_H_

