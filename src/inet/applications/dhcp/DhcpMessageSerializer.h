//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DHCPMESSAGESERIALIZER_H
#define __INET_DHCPMESSAGESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between DhcpMessage and binary (network byte order) DHCP message.
 */
class INET_API DhcpMessageSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    DhcpMessageSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

