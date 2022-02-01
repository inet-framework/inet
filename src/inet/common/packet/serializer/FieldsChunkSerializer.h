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
     * chunk at the end of the stream.
     */
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const = 0;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The current stream position is updated according
     * to the length of the returned chunk.
     */
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const = 0;

  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace

#endif

