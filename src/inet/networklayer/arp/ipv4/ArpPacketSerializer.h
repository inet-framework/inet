//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ARPPACKETSERIALIZER_H
#define __INET_ARPPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between ArpPacket and binary (network byte order)  Arp header.
 */
class INET_API ArpPacketSerializer : public FieldsChunkSerializer
{
  protected:
    MacAddress readMacAddress(MemoryInputStream& stream, unsigned int size) const;
    Ipv4Address readIpv4Address(MemoryInputStream& stream, unsigned int size) const;

    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    ArpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif

