//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKPHYHEADERSERIALIZER_H
#define __INET_APSKPHYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/ChunkSerializer.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"

namespace inet {

namespace physicallayer {

class INET_API ApskPhyHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const;
};

} // namespace physicallayer

} // namespace inet

#endif

