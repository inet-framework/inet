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

#include "inet/common/packet/ByteInputStream.h"
#include "inet/common/packet/ByteOutputStream.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

class INET_API ChunkSerializer : public cObject
{
  public:
    static bit totalSerializedBitCount;
    static bit totalDeserializedBitCount;

  public:
    virtual ~ChunkSerializer() { }

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const = 0;
};

class INET_API BitCountChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API BitsChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API ByteCountChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API BytesChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API SliceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API SequenceChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

class INET_API FieldsChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const = 0;

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const;
};

} // namespace

#endif // #ifndef __INET_SERIALIZER_H_

