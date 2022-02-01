//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021DBPDUSERIALIZER_H
#define __INET_IEEE8021DBPDUSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between Bpdu and binary (network byte order) Ieee 802.1d BPDU packets.
 */
class INET_API Ieee8021dBpduSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee8021dBpduSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

