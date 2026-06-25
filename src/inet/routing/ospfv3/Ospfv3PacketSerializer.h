//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSPFV3PACKETSERIALIZER_H
#define __INET_OSPFV3PACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

namespace inet {

namespace ospf {
class OspfPacketSerializer;
} // namespace ospf

namespace ospfv3 {

/**
 * Converts between Ospfv3Packet and binary (network byte order) OSPFv3 data
 * (RFC 5340 Appendix A).
 */
class INET_API Ospfv3PacketSerializer : public FieldsChunkSerializer
{
  private:
    static void serializeOspfHeader(MemoryOutputStream& stream, const Ptr<const Ospfv3Packet>& ospfPacket);
    static uint16_t deserializeOspfHeader(MemoryInputStream& stream, Ptr<Ospfv3Packet>& ospfPacket);

    static void serializeOspfOptions(MemoryOutputStream& stream, const Ospfv3Options& options);
    static void deserializeOspfOptions(MemoryInputStream& stream, Ospfv3Options& options);

    static void serializeLsaHeader(MemoryOutputStream& stream, const Ospfv3LsaHeader& lsaHeader);
    static void deserializeLsaHeader(MemoryInputStream& stream, Ospfv3LsaHeader& lsaHeader);

    // a full 128-bit (16-octet) IPv6 address, per RFC 5340 -- used for the Link-LSA
    // Link-local Interface Address (A.4.8); address *prefixes* are variable-length
    // (see serializePrefixAddress)
    static void serializeFixedAddress(MemoryOutputStream& stream, const L3Address& addr);
    static L3Address deserializeFixedAddress(MemoryInputStream& stream);

    // an RFC 5340 A.4.1 variable-length address prefix: ((prefixLen + 31) / 32) 32-bit words
    static void serializePrefixAddress(MemoryOutputStream& stream, const L3Address& addr, uint8_t prefixLen);
    static L3Address deserializePrefixAddress(MemoryInputStream& stream, uint8_t prefixLen);

    // an address-prefix encoding (RFC 5340 A.4.1): 4-octet fixed part + variable-length address
    static void serializePrefix0(MemoryOutputStream& stream, const Ospfv3LsaPrefix0& prefix);
    static void deserializePrefix0(MemoryInputStream& stream, Ospfv3LsaPrefix0& prefix);
    static void serializePrefixMetric(MemoryOutputStream& stream, const Ospfv3LsaPrefixMetric& prefix);
    static void deserializePrefixMetric(MemoryInputStream& stream, Ospfv3LsaPrefixMetric& prefix);

    static void serializeRouterLsa(MemoryOutputStream& stream, const Ospfv3RouterLsa& lsa);
    static void deserializeRouterLsa(MemoryInputStream& stream, Ospfv3RouterLsa& lsa);
    static void serializeNetworkLsa(MemoryOutputStream& stream, const Ospfv3NetworkLsa& lsa);
    static void deserializeNetworkLsa(MemoryInputStream& stream, Ospfv3NetworkLsa& lsa);
    static void serializeInterAreaPrefixLsa(MemoryOutputStream& stream, const Ospfv3InterAreaPrefixLsa& lsa);
    static void deserializeInterAreaPrefixLsa(MemoryInputStream& stream, Ospfv3InterAreaPrefixLsa& lsa);
    static void serializeLinkLsa(MemoryOutputStream& stream, const Ospfv3LinkLsa& lsa);
    static void deserializeLinkLsa(MemoryInputStream& stream, Ospfv3LinkLsa& lsa);
    static void serializeIntraAreaPrefixLsa(MemoryOutputStream& stream, const Ospfv3IntraAreaPrefixLsa& lsa);
    static void deserializeIntraAreaPrefixLsa(MemoryInputStream& stream, Ospfv3IntraAreaPrefixLsa& lsa);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ospfv3PacketSerializer() : FieldsChunkSerializer() {}

    // Serializes a single LSA (header + body) to the stream. Exposed so setLsaChecksum() (see
    // Ospfv3Checksum) can compute the RFC 2328 / RFC 5340 LS Checksum (Fletcher) over a finalized LSA.
    static void serializeLsa(MemoryOutputStream& stream, const Ospfv3Lsa& lsa);

    friend class inet::ospf::OspfPacketSerializer;
};

} // namespace ospfv3
} // namespace inet

#endif
