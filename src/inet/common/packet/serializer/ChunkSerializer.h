//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHUNKSERIALIZER_H
#define __INET_CHUNKSERIALIZER_H

#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

class INET_API ChunkSerializer : public cObject
{
  public:
    static OPP_THREAD_LOCAL b totalSerializedLength;
    static OPP_THREAD_LOCAL b totalDeserializedLength;

  public:
    /**
     * Serializes a chunk into a stream by writing the bytes representing the
     * chunk at the end of the stream. The offset and length parameters allow
     * to write only a part of the data.
     */
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const = 0;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The returned chunk will be an instance of the
     * provided type. The current stream position is updated according to the
     * length of the returned chunk.
     */
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const = 0;
};

} // namespace

#endif

