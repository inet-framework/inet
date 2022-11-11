//
// Copyright (C) 2004 OpenSim Ltd.
//               2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ICMPV6HEADERSERIALIZER_H
#define __INET_ICMPV6HEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between Icmpv6Header and binary (network byte order) ICMPv6 header.
 */
class INET_API Icmpv6HeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Icmpv6HeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

