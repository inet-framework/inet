//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AODVCONTROLPACKETSSERIALIZER_H
#define __INET_AODVCONTROLPACKETSSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace aodv {

/**
 * Converts between AodvControlPackets and binary (network byte order) Aodv control packets.
 */
class INET_API AodvControlPacketsSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    AodvControlPacketsSerializer() : FieldsChunkSerializer() {}
};

} // namespace aodv
} // namespace inet

#endif

