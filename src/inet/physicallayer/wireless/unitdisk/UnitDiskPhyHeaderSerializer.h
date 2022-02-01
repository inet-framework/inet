//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKPHYHEADERSERIALIZER_H
#define __INET_UNITDISKPHYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between UnitDiskPhyHeader and binary (network byte order) application packet.
 */
class INET_API UnitDiskPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    UnitDiskPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

