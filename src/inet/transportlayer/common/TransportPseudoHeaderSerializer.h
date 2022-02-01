//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTPSEUDOHEADERSERIALIZER_H
#define __INET_TRANSPORTPSEUDOHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between TransportPseudoHeader and binary (network byte order) transport pseudo header.
 */
class INET_API TransportPseudoHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    TransportPseudoHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

