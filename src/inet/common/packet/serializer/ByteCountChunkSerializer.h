//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BYTECOUNTCHUNKSERIALIZER_H
#define __INET_BYTECOUNTCHUNKSERIALIZER_H

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

class INET_API ByteCountChunkSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace

#endif

