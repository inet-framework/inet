//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FIELDSCHUNKSERIALIZER_H
#define __INET_FIELDSCHUNKSERIALIZER_H

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

class INET_API FieldsChunkSerializer : public ChunkSerializer
{
  protected:
    /**
     * Serializes a chunk into a stream by writing all bytes representing the
     * chunk at the end of the stream. A subclass overrides either this hook or
     * serializeFields(). The default implementation throws an error.
     */
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The current stream position is updated according
     * to the length of the returned chunk. A subclass overrides either this hook
     * or deserializeFields(). The default implementation throws an error.
     */
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const;

    /**
     * Serializes a chunk into a stream by writing all bytes representing the
     * chunk at the end of the stream. The default implementation calls the
     * serialize(MemoryOutputStream&, const Ptr<const Chunk>&) hook.
     */
    virtual void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The typeInfo parameter specifies the concrete
     * chunk type requested from ChunkSerializerRegistry. A serializer that is
     * registered for several chunk types can inspect typeInfo to build the
     * exact requested type. The current stream position is updated according
     * to the length of the returned chunk. The default implementation calls
     * the deserialize(MemoryInputStream&) hook, which ignores the type.
     */
    virtual const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const;

  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace

#endif

