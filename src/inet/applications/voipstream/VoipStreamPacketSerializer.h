//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_VOIPSTREAMPACKETSERIALIZER_H
#define __INET_VOIPSTREAMPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between VoipStreamPacket and binary (network byte order) Udp header.
 */
class INET_API VoipStreamPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    VoipStreamPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

