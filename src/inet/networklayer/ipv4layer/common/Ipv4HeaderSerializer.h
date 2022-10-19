//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4HEADERSERIALIZER_H
#define __INET_IPV4HEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

/**
 * Converts between Ipv4Header and binary (network byte order) Ipv4 header.
 */
class INET_API Ipv4HeaderSerializer : public FieldsChunkSerializer
{
  protected:
    static void serializeOption(MemoryOutputStream& stream, const TlvOptionBase *option);
    virtual TlvOptionBase *deserializeOption(MemoryInputStream& stream) const;

    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;


  public:
    Ipv4HeaderSerializer() : FieldsChunkSerializer() {}
    static void serialize(MemoryOutputStream& stream, const Ipv4Header& ipv4Header);
};

} // namespace inet

#endif

