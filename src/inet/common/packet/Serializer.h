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

#include "inet/common/packet/BytesChunk.h"
#include "inet/common/packet/ByteStream.h"
#include "inet/common/packet/LengthChunk.h"
#include "inet/common/packet/SequenceChunk.h"
#include "inet/common/packet/SliceChunk.h"

namespace inet {

#define Register_Serializer(CLASSNAME) Register_Class(CLASSNAME)

class ChunkSerializer : public cObject
{
  public:
    static int64_t totalSerializedLength;
    static int64_t totalDeserializedLength;

  public:
    virtual ~ChunkSerializer() { }

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const = 0;
};

class BytesChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class LengthChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class SliceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class SequenceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class FieldsChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const = 0;

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

} // namespace

#endif // #ifndef __INET_SERIALIZER_H_

