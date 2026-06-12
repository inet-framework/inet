//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MOBILITYHEADERSERIALIZER_H
#define __INET_MOBILITYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between MobilityHeader (and subclasses) and binary (network byte order)
 * MIPv6 Mobility Header as defined in RFC 6275 Section 6.1.
 */
class INET_API MobilityHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    MobilityHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
