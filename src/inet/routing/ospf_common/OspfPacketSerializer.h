//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSPFPACKETSERIALIZER_H
#define __INET_OSPFPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace ospf {

/**
 * Converts between OspfPacket and binary (network byte order) OSPF data.
 */
class INET_API OspfPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    OspfPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace ospf
} // namespace inet

#endif

