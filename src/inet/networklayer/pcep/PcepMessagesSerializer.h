//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCEPMESSAGESSERIALIZER_H
#define __INET_PCEPMESSAGESSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/pcep/PcepMessages_m.h"

namespace inet {

/**
 * Converts between PcepMessage subtypes and binary (network byte order) PCEP
 * messages, per RFC 5440. Phase 1 of this workstream: Common Header (Section 6.1)
 * plus the Open (Section 6.3/7.3) and Keepalive (Section 6.4) messages only.
 *
 * Unlike ~LdpPacketSerializer's PDU/message-length convention (which excludes the
 * length field's own bytes), RFC 5440's Message-Length (Common Header) and
 * Object-Length (object header) are both INCLUSIVE of their own header -- see
 * serialize()/deserialize() for the exact byte accounting.
 */
class INET_API PcepMessagesSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    PcepMessagesSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
