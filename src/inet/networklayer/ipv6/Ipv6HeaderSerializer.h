//
// Copyright (C) 2013 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6HEADERSERIALIZER_H
#define __INET_IPV6HEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between IPv6Datagram and binary (network byte order) Ipv6 header.
 */
class INET_API Ipv6HeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ipv6HeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

