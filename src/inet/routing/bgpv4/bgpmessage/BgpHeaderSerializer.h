//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BGPHEADERSERIALIZER_H
#define __INET_BGPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace bgp {

/**
 * Converts between BgpHeader and binary (network byte order) BGP messages.
 */
class INET_API BgpHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    BgpHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace bgp
} // namespace inet

#endif

