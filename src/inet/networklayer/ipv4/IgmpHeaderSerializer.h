//
// Copyright (C) 2004 OpenSim Ltd.
//               2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IGMPHEADERSERIALIZER_H
#define __INET_IGMPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between IgmpMessage and binary (network byte order) IGMP message.
 */
class INET_API IgmpHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    IgmpHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

