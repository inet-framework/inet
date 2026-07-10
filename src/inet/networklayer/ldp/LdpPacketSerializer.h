//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LDPPACKETSERIALIZER_H
#define __INET_LDPPACKETSERIALIZER_H

#include <vector>

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

    // RFC 5036 Section 2.8/3.4.4-3.4.5: optional Hop Count + Path Vector TLVs,
    // carried by Label Request/Mapping messages only when Ldp.loopDetection is
    // enabled. Presence is NOT inferred from how much data remains in the
    // underlying stream (this message need not be the last one in the byte
    // stream/queue) -- callers determine presence from the message's own
    // Message Length field (see deserialize()) before calling
    // deserializeLoopDetectionTlvs().
    static void serializeLoopDetectionTlvs(MemoryOutputStream& stream, uint8_t hopCount, const std::vector<Ipv4Address>& pathVector);
    static void deserializeLoopDetectionTlvs(MemoryInputStream& stream, uint8_t& hopCount, std::vector<Ipv4Address>& pathVector);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    LdpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif
