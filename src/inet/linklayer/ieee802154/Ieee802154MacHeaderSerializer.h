//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154MACHEADERSERIALIZER_H
#define __INET_IEEE802154MACHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between Ieee802154MacHeader and binary network byte order packet.
 */
class INET_API Ieee802154MacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee802154MacHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif // __INET_IEEE802154MACHEADERSERIALIZER_H
