//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GENERICPHYHEADERSERIALIZER_H
#define __INET_GENERICPHYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between GenericPhyHeader and binary (network byte order) application packet.
 */
class INET_API GenericPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    GenericPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

