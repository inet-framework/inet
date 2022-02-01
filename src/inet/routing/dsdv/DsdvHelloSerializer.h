//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DSDVHELLOSERIALIZER_H
#define __INET_DSDVHELLOSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between DsdvHello and binary (network byte order) dsdv hello packet.
 */
class INET_API DsdvHelloSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    DsdvHelloSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

