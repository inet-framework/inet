//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_APPLICATIONPACKETSERIALIZER_H
#define __INET_APPLICATIONPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between ApplicationPacket and binary (network byte order) application packet.
 */
class INET_API ApplicationPacketSerializer : public FieldsChunkSerializer
{
  public:
    ApplicationPacketSerializer() = default;

  protected:
    void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};

} // namespace inet

#endif // __INET_APPLICATIONPACKETSERIALIZER_H

