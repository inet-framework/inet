//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LDPPACKETSERIALIZER_H
#define __INET_LDPPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/ldp/LdpPacket_m.h"

namespace inet {

/**
 * Converts between LdpPacket subtypes and binary (network byte order) LDP PDUs,
 * per RFC 5036. Model simplification (documented in Ldp.ned): this model always
 * carries exactly one message per PDU, so the PDU header and the single
 * message's header are both handled by this one serializer.
 *
 * The PDU Length and Message Length wire fields are derived from the chunk's
 * actual length on serialize, and used to reconstruct it on deserialize --
 * they are not separately stored msg fields (see LdpPacket.msg).
 */
class INET_API LdpPacketSerializer : public FieldsChunkSerializer
{
  private:
    // RFC 5036 Section 3.4.1: FEC TLV (type 0x0100) carrying a single
    // Address Prefix FEC Element (element type 2, address family 1/IP).
    static void serializeFecTlv(MemoryOutputStream& stream, const FecTlv& fec);
    static FecTlv deserializeFecTlv(MemoryInputStream& stream);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    LdpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
