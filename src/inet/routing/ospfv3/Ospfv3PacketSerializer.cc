//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospfv3/Ospfv3PacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"

namespace inet {
namespace ospfv3 {

using namespace ospf;

Register_Serializer(Ospfv3Packet, Ospfv3PacketSerializer);
Register_Serializer(Ospfv3HelloPacket, Ospfv3PacketSerializer);
Register_Serializer(Ospfv3DatabaseDescriptionPacket, Ospfv3PacketSerializer);
Register_Serializer(Ospfv3LinkStateRequestPacket, Ospfv3PacketSerializer);
Register_Serializer(Ospfv3LinkStateUpdatePacket, Ospfv3PacketSerializer);
Register_Serializer(Ospfv3LinkStateAcknowledgementPacket, Ospfv3PacketSerializer);

// ---- common OSPFv3 packet header (RFC 5340 A.3.1, 16 octets) ----

void Ospfv3PacketSerializer::serializeOspfHeader(MemoryOutputStream& stream, const Ptr<const Ospfv3Packet>& ospfPacket)
{
    stream.writeByte(ospfPacket->getVersion());
    stream.writeByte(ospfPacket->getType());
    stream.writeUint16Be(ospfPacket->getPacketLengthField());
    stream.writeIpv4Address(ospfPacket->getRouterID());
    stream.writeIpv4Address(ospfPacket->getAreaID());
    stream.writeUint16Be(ospfPacket->getChecksum());
    stream.writeByte(ospfPacket->getInstanceID());
    stream.writeByte(ospfPacket->getReserved());
}

uint16_t Ospfv3PacketSerializer::deserializeOspfHeader(MemoryInputStream& stream, Ptr<Ospfv3Packet>& ospfPacket)
{
    int ospfVer = stream.readUint8();
    if (ospfVer != 3)
        ospfPacket->markIncorrect();
    ospfPacket->setVersion(ospfVer);
    int ospfType = stream.readUint8();
    ospfPacket->setType(static_cast<OspfPacketType>(ospfType));
    uint16_t packetLength = stream.readUint16Be();
    ospfPacket->setPacketLengthField(packetLength);
    ospfPacket->setChunkLength(B(packetLength));
    ospfPacket->setRouterID(stream.readIpv4Address());
    ospfPacket->setAreaID(stream.readIpv4Address());
    ospfPacket->setChecksum(stream.readUint16Be());
    ospfPacket->setChecksumMode(CHECKSUM_COMPUTED);
    ospfPacket->setInstanceID(stream.readByte());
    ospfPacket->setReserved(stream.readByte());
    return packetLength;
}

// ---- options (RFC 5340 A.2, 24 bits) ----

void Ospfv3PacketSerializer::serializeOspfOptions(MemoryOutputStream& stream, const Ospfv3Options& options)
{
    stream.writeUint16Be(options.reserved);
    stream.writeBit(options.reservedOne);
    stream.writeBit(options.reservedTwo);
    stream.writeBit(options.dcBit);
    stream.writeBit(options.rBit);
    stream.writeBit(options.nBit);
    stream.writeBit(options.xBit);
    stream.writeBit(options.eBit);
    stream.writeBit(options.v6Bit);
}

void Ospfv3PacketSerializer::deserializeOspfOptions(MemoryInputStream& stream, Ospfv3Options& options)
{
    options.reserved = stream.readUint16Be();
    options.reservedOne = stream.readBit();
    options.reservedTwo = stream.readBit();
    options.dcBit = stream.readBit();
    options.rBit = stream.readBit();
    options.nBit = stream.readBit();
    options.xBit = stream.readBit();
    options.eBit = stream.readBit();
    options.v6Bit = stream.readBit();
}

// ---- LSA header (RFC 5340 A.4.2, 20 octets) ----

// RFC 5340 A.4.2.1: the LSA "LS Type" is a 16-bit field = U bit | S2 S1 (flooding scope) |
// 13-bit function code. The model stores only the function code (ROUTER_LSA=1, NETWORK_LSA=2,
// ...) in lsaType; the flooding scope is well known per LSA type. Encode the full 16-bit value
// so the wire is RFC-compliant (Router-LSA=0x2001, Network-LSA=0x2002, Inter-Area-Prefix=0x2003,
// AS-External=0x4005, Link-LSA=0x0008, Intra-Area-Prefix=0x2009).
static uint16_t encodeLsType(uint16_t functionCode)
{
    uint16_t scope;
    switch (functionCode & 0x1fff) {
        case LINK_LSA:        scope = 0x0000; break; // link-local scope (S2 S1 = 00)
        case AS_EXTERNAL_LSA: scope = 0x4000; break; // AS scope (S2 S1 = 10)
        default:              scope = 0x2000; break; // area scope (S2 S1 = 01)
    }
    return scope | (functionCode & 0x1fff);
}

void Ospfv3PacketSerializer::serializeLsaHeader(MemoryOutputStream& stream, const Ospfv3LsaHeader& lsaHeader)
{
    stream.writeUint16Be(lsaHeader.getLsaAge());
    stream.writeUint16Be(encodeLsType(lsaHeader.getLsaType()));
    stream.writeIpv4Address(lsaHeader.getLinkStateID());
    stream.writeIpv4Address(lsaHeader.getAdvertisingRouter());
    stream.writeUint32Be(lsaHeader.getLsaSequenceNumber());
    stream.writeUint16Be(lsaHeader.getLsaChecksum());
    stream.writeUint16Be(lsaHeader.getLsaLength());
}

void Ospfv3PacketSerializer::deserializeLsaHeader(MemoryInputStream& stream, Ospfv3LsaHeader& lsaHeader)
{
    lsaHeader.setLsaAge(stream.readUint16Be());
    uint16_t lsType = stream.readUint16Be();
    lsaHeader.setOptions((lsType >> 8) & 0xff); // U + S2 S1 scope bits (high byte)
    lsaHeader.setLsaType(lsType & 0xff);        // function code (low byte)
    lsaHeader.setLinkStateID(stream.readIpv4Address());
    lsaHeader.setAdvertisingRouter(stream.readIpv4Address());
    lsaHeader.setLsaSequenceNumber(stream.readUint32Be());
    lsaHeader.setLsaChecksum(stream.readUint16Be());
    lsaHeader.setLsaLength(stream.readUint16Be());
    lsaHeader.setLsChecksumMode(CHECKSUM_COMPUTED);
}

// A full 128-bit (16-octet) IPv6 address, exactly as RFC 5340 encodes genuine address fields.
// The only such field in the supported LSAs is the Link-LSA "Link-local Interface Address"
// (A.4.8), which is always a 128-bit IPv6 address -- this is the RFC-compliant on-the-wire
// encoding for it. (Address *prefixes* are NOT fixed-size; they use the variable-length
// encoding in serializePrefixAddress.) The model keeps addresses in L3Address fields; an
// IPv6 instance writes the address verbatim, the (non-default) IPv4 address family falls back
// to the 4-octet address zero-padded to 16, and an unset field to all zeroes.
void Ospfv3PacketSerializer::serializeFixedAddress(MemoryOutputStream& stream, const L3Address& addr)
{
    if (addr.getType() == L3Address::IPv6)
        stream.writeIpv6Address(addr.toIpv6());
    else if (addr.getType() == L3Address::IPv4) {
        stream.writeIpv4Address(addr.toIpv4());
        stream.writeByteRepeatedly(0, 12);
    }
    else
        stream.writeByteRepeatedly(0, 16);
}

L3Address Ospfv3PacketSerializer::deserializeFixedAddress(MemoryInputStream& stream)
{
    return L3Address(stream.readIpv6Address());
}

// An "address prefix" (RFC 5340 A.4.1): only the significant high-order bits of the
// address are carried, as an even multiple of 32-bit words, i.e. ((PrefixLength + 31) / 32)
// words, padding with zero bits. This is the RFC-compliant on-the-wire encoding (so e.g.
// Wireshark can dissect the trace), and it matches the length computed by calculateLSASize().
void Ospfv3PacketSerializer::serializePrefixAddress(MemoryOutputStream& stream, const L3Address& addr, uint8_t prefixLen)
{
    int numWords = (prefixLen + 31) / 32;
    if (numWords > 4)
        numWords = 4;
    uint32_t words[4] = { 0, 0, 0, 0 };
    if (addr.getType() == L3Address::IPv6) {
        const uint32_t *w = addr.toIpv6().words();
        for (int i = 0; i < 4; ++i)
            words[i] = w[i];
    }
    else if (addr.getType() == L3Address::IPv4)
        words[0] = addr.toIpv4().getInt();
    for (int i = 0; i < numWords; ++i)
        stream.writeUint32Be(words[i]);
}

L3Address Ospfv3PacketSerializer::deserializePrefixAddress(MemoryInputStream& stream, uint8_t prefixLen)
{
    int numWords = (prefixLen + 31) / 32;
    if (numWords > 4)
        numWords = 4;
    uint32_t words[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < numWords; ++i)
        words[i] = stream.readUint32Be();
    return L3Address(Ipv6Address(words[0], words[1], words[2], words[3]));
}

// ---- address prefixes (RFC 5340 A.4.1): 4-octet fixed part + variable-length address ----

void Ospfv3PacketSerializer::serializePrefix0(MemoryOutputStream& stream, const Ospfv3LsaPrefix0& prefix)
{
    stream.writeByte(prefix.prefixLen);
    stream.writeBit(prefix.reserved1);
    stream.writeBit(prefix.reserved2);
    stream.writeBit(prefix.reserved3);
    stream.writeBit(prefix.dnBit);
    stream.writeBit(prefix.pBit);
    stream.writeBit(prefix.xBit);
    stream.writeBit(prefix.laBit);
    stream.writeBit(prefix.nuBit);
    stream.writeUint16Be(prefix.reserved);
    serializePrefixAddress(stream, prefix.addressPrefix, prefix.prefixLen);
}

void Ospfv3PacketSerializer::deserializePrefix0(MemoryInputStream& stream, Ospfv3LsaPrefix0& prefix)
{
    prefix.prefixLen = stream.readByte();
    prefix.reserved1 = stream.readBit();
    prefix.reserved2 = stream.readBit();
    prefix.reserved3 = stream.readBit();
    prefix.dnBit = stream.readBit();
    prefix.pBit = stream.readBit();
    prefix.xBit = stream.readBit();
    prefix.laBit = stream.readBit();
    prefix.nuBit = stream.readBit();
    prefix.reserved = stream.readUint16Be();
    prefix.addressPrefix = deserializePrefixAddress(stream, prefix.prefixLen);
}

void Ospfv3PacketSerializer::serializePrefixMetric(MemoryOutputStream& stream, const Ospfv3LsaPrefixMetric& prefix)
{
    stream.writeByte(prefix.prefixLen);
    stream.writeBit(prefix.reserved1);
    stream.writeBit(prefix.reserved2);
    stream.writeBit(prefix.reserved3);
    stream.writeBit(prefix.dnBit);
    stream.writeBit(prefix.pBit);
    stream.writeBit(prefix.xBit);
    stream.writeBit(prefix.laBit);
    stream.writeBit(prefix.nuBit);
    stream.writeUint16Be(prefix.metric);
    serializePrefixAddress(stream, prefix.addressPrefix, prefix.prefixLen);
}

void Ospfv3PacketSerializer::deserializePrefixMetric(MemoryInputStream& stream, Ospfv3LsaPrefixMetric& prefix)
{
    prefix.prefixLen = stream.readByte();
    prefix.reserved1 = stream.readBit();
    prefix.reserved2 = stream.readBit();
    prefix.reserved3 = stream.readBit();
    prefix.dnBit = stream.readBit();
    prefix.pBit = stream.readBit();
    prefix.xBit = stream.readBit();
    prefix.laBit = stream.readBit();
    prefix.nuBit = stream.readBit();
    prefix.metric = stream.readUint16Be();
    prefix.addressPrefix = deserializePrefixAddress(stream, prefix.prefixLen);
}

// ---- LSA bodies (RFC 5340 A.4.3 - A.4.9) ----

void Ospfv3PacketSerializer::serializeRouterLsa(MemoryOutputStream& stream, const Ospfv3RouterLsa& lsa)
{
    stream.writeNBitsOfUint64Be(0, 3);
    stream.writeBit(lsa.getNtBit());
    stream.writeBit(lsa.getXBit());
    stream.writeBit(lsa.getVBit());
    stream.writeBit(lsa.getEBit());
    stream.writeBit(lsa.getBBit());
    serializeOspfOptions(stream, lsa.getOspfOptions());
    for (size_t i = 0; i < lsa.getRoutersArraySize(); ++i) {
        const auto& r = lsa.getRouters(i);
        stream.writeByte(r.type);
        stream.writeByte(0);
        stream.writeUint16Be(r.metric);
        stream.writeUint32Be(r.interfaceID);
        stream.writeUint32Be(r.neighborInterfaceID);
        stream.writeIpv4Address(r.neighborRouterID);
    }
}

void Ospfv3PacketSerializer::deserializeRouterLsa(MemoryInputStream& stream, Ospfv3RouterLsa& lsa)
{
    stream.readNBitsToUint64Be(3);
    lsa.setNtBit(stream.readBit());
    lsa.setXBit(stream.readBit());
    lsa.setVBit(stream.readBit());
    lsa.setEBit(stream.readBit());
    lsa.setBBit(stream.readBit());
    deserializeOspfOptions(stream, lsa.getOspfOptionsForUpdate());
    int numRouters = (B(lsa.getHeader().getLsaLength()) - OSPFV3_LSA_HEADER_LENGTH - OSPFV3_ROUTER_LSA_HEADER_LENGTH).get<B>() / OSPFV3_ROUTER_LSA_BODY_LENGTH.get<B>();
    if (numRouters < 0)
        numRouters = 0;
    lsa.setRoutersArraySize(numRouters);
    for (int i = 0; i < numRouters; ++i) {
        Ospfv3RouterLsaBody r;
        r.type = stream.readByte();
        stream.readByte();
        r.metric = stream.readUint16Be();
        r.interfaceID = stream.readUint32Be();
        r.neighborInterfaceID = stream.readUint32Be();
        r.neighborRouterID = stream.readIpv4Address();
        lsa.setRouters(i, r);
    }
}

void Ospfv3PacketSerializer::serializeNetworkLsa(MemoryOutputStream& stream, const Ospfv3NetworkLsa& lsa)
{
    stream.writeByte(0);
    serializeOspfOptions(stream, lsa.getOspfOptions());
    for (size_t i = 0; i < lsa.getAttachedRouterArraySize(); ++i)
        stream.writeIpv4Address(lsa.getAttachedRouter(i));
}

void Ospfv3PacketSerializer::deserializeNetworkLsa(MemoryInputStream& stream, Ospfv3NetworkLsa& lsa)
{
    stream.readByte();
    deserializeOspfOptions(stream, lsa.getOspfOptionsForUpdate());
    int numAttached = (B(lsa.getHeader().getLsaLength()) - OSPFV3_LSA_HEADER_LENGTH - OSPFV3_NETWORK_LSA_HEADER_LENGTH).get<B>() / OSPFV3_NETWORK_LSA_ATTACHED_LENGTH.get<B>();
    if (numAttached < 0)
        numAttached = 0;
    lsa.setAttachedRouterArraySize(numAttached);
    for (int i = 0; i < numAttached; ++i)
        lsa.setAttachedRouter(i, stream.readIpv4Address());
}

void Ospfv3PacketSerializer::serializeInterAreaPrefixLsa(MemoryOutputStream& stream, const Ospfv3InterAreaPrefixLsa& lsa)
{
    stream.writeByte(lsa.getReserved1());
    stream.writeUint24Be(lsa.getMetric());
    serializePrefix0(stream, lsa.getPrefix());
}

void Ospfv3PacketSerializer::deserializeInterAreaPrefixLsa(MemoryInputStream& stream, Ospfv3InterAreaPrefixLsa& lsa)
{
    lsa.setReserved1(stream.readByte());
    lsa.setMetric(stream.readUint24Be());
    deserializePrefix0(stream, lsa.getPrefixForUpdate());
}

void Ospfv3PacketSerializer::serializeLinkLsa(MemoryOutputStream& stream, const Ospfv3LinkLsa& lsa)
{
    stream.writeByte(lsa.getRouterPriority());
    serializeOspfOptions(stream, lsa.getOspfOptions());
    serializeFixedAddress(stream, lsa.getLinkLocalInterfaceAdd());
    stream.writeUint32Be(lsa.getNumPrefixes());
    for (size_t i = 0; i < lsa.getPrefixesArraySize(); ++i)
        serializePrefix0(stream, lsa.getPrefixes(i));
}

void Ospfv3PacketSerializer::deserializeLinkLsa(MemoryInputStream& stream, Ospfv3LinkLsa& lsa)
{
    lsa.setRouterPriority(stream.readByte());
    deserializeOspfOptions(stream, lsa.getOspfOptionsForUpdate());
    lsa.setLinkLocalInterfaceAdd(deserializeFixedAddress(stream));
    uint32_t numPrefixes = stream.readUint32Be();
    lsa.setNumPrefixes(numPrefixes);
    lsa.setPrefixesArraySize(numPrefixes);
    for (uint32_t i = 0; i < numPrefixes; ++i) {
        Ospfv3LsaPrefix0 prefix;
        deserializePrefix0(stream, prefix);
        lsa.setPrefixes(i, prefix);
    }
}

void Ospfv3PacketSerializer::serializeIntraAreaPrefixLsa(MemoryOutputStream& stream, const Ospfv3IntraAreaPrefixLsa& lsa)
{
    stream.writeUint16Be(lsa.getNumPrefixes());
    stream.writeUint16Be(lsa.getReferencedLSType());
    stream.writeIpv4Address(lsa.getReferencedLSID());
    stream.writeIpv4Address(lsa.getReferencedAdvRtr());
    for (size_t i = 0; i < lsa.getPrefixesArraySize(); ++i)
        serializePrefixMetric(stream, lsa.getPrefixes(i));
}

void Ospfv3PacketSerializer::deserializeIntraAreaPrefixLsa(MemoryInputStream& stream, Ospfv3IntraAreaPrefixLsa& lsa)
{
    uint16_t numPrefixes = stream.readUint16Be();
    lsa.setNumPrefixes(numPrefixes);
    lsa.setReferencedLSType(stream.readUint16Be());
    lsa.setReferencedLSID(stream.readIpv4Address());
    lsa.setReferencedAdvRtr(stream.readIpv4Address());
    lsa.setPrefixesArraySize(numPrefixes);
    for (uint16_t i = 0; i < numPrefixes; ++i) {
        Ospfv3LsaPrefixMetric prefix;
        deserializePrefixMetric(stream, prefix);
        lsa.setPrefixes(i, prefix);
    }
}

void Ospfv3PacketSerializer::serializeLsa(MemoryOutputStream& stream, const Ospfv3Lsa& lsa)
{
    serializeLsaHeader(stream, lsa.getHeader());
    switch (lsa.getHeader().getLsaType() & 0x1f) {
        case ROUTER_LSA:
            serializeRouterLsa(stream, *static_cast<const Ospfv3RouterLsa *>(&lsa));
            break;
        case NETWORK_LSA:
            serializeNetworkLsa(stream, *static_cast<const Ospfv3NetworkLsa *>(&lsa));
            break;
        case INTER_AREA_PREFIX_LSA:
            serializeInterAreaPrefixLsa(stream, *static_cast<const Ospfv3InterAreaPrefixLsa *>(&lsa));
            break;
        case LINK_LSA:
            serializeLinkLsa(stream, *static_cast<const Ospfv3LinkLsa *>(&lsa));
            break;
        case INTRA_AREA_PREFIX_LSA:
            serializeIntraAreaPrefixLsa(stream, *static_cast<const Ospfv3IntraAreaPrefixLsa *>(&lsa));
            break;
        default:
            throw cRuntimeError("Cannot serialize OSPFv3 LSA: function code %d not supported", lsa.getHeader().getLsaType() & 0x1f);
    }
}

// ---- packets ----

void Ospfv3PacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ospfPacket = staticPtrCast<const Ospfv3Packet>(chunk);
    serializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            const auto& hello = staticPtrCast<const Ospfv3HelloPacket>(ospfPacket);
            stream.writeUint32Be(hello->getInterfaceID());
            stream.writeByte(hello->getRouterPriority());
            serializeOspfOptions(stream, hello->getOptions());
            stream.writeUint16Be(hello->getHelloInterval());
            stream.writeUint16Be(hello->getDeadInterval());
            stream.writeIpv4Address(hello->getDesignatedRouterID());
            stream.writeIpv4Address(hello->getBackupDesignatedRouterID());
            for (size_t i = 0; i < hello->getNeighborIDArraySize(); ++i)
                stream.writeIpv4Address(hello->getNeighborID(i));
            break;
        }
        case DATABASE_DESCRIPTION_PACKET: {
            const auto& dd = staticPtrCast<const Ospfv3DatabaseDescriptionPacket>(ospfPacket);
            stream.writeByte(dd->getReserved1());
            serializeOspfOptions(stream, dd->getOptions());
            stream.writeUint16Be(dd->getInterfaceMTU());
            const auto& ddo = dd->getDdOptions();
            stream.writeNBitsOfUint64Be(ddo.reserved, 13);
            stream.writeBit(ddo.iBit);
            stream.writeBit(ddo.mBit);
            stream.writeBit(ddo.msBit);
            stream.writeUint32Be(dd->getSequenceNumber());
            for (size_t i = 0; i < dd->getLsaHeadersArraySize(); ++i)
                serializeLsaHeader(stream, dd->getLsaHeaders(i));
            break;
        }
        case LINKSTATE_REQUEST_PACKET: {
            const auto& req = staticPtrCast<const Ospfv3LinkStateRequestPacket>(ospfPacket);
            for (size_t i = 0; i < req->getRequestsArraySize(); ++i) {
                const auto& r = req->getRequests(i);
                stream.writeUint16Be(0);
                stream.writeUint16Be(r.lsaType);
                stream.writeIpv4Address(r.lsaID);
                stream.writeIpv4Address(r.advertisingRouter);
            }
            break;
        }
        case LINKSTATE_UPDATE_PACKET: {
            const auto& upd = staticPtrCast<const Ospfv3LinkStateUpdatePacket>(ospfPacket);
            stream.writeUint32Be(upd->getLsaCount());
            for (size_t i = 0; i < upd->getLsasArraySize(); ++i)
                serializeLsa(stream, *upd->getLsas(i));
            break;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            const auto& ack = staticPtrCast<const Ospfv3LinkStateAcknowledgementPacket>(ospfPacket);
            for (size_t i = 0; i < ack->getLsaHeadersArraySize(); ++i)
                serializeLsaHeader(stream, ack->getLsaHeaders(i));
            break;
        }
        default:
            throw cRuntimeError("Unknown OSPFv3 message type %d in Ospfv3PacketSerializer", ospfPacket->getType());
    }
}

const Ptr<Chunk> Ospfv3PacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ospfPacket = makeShared<Ospfv3Packet>();
    uint16_t packetLength = deserializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            auto hello = makeShared<Ospfv3HelloPacket>();
            hello->setChunkLength(B(packetLength));
            hello->setVersion(ospfPacket->getVersion());
            hello->setType(ospfPacket->getType());
            hello->setPacketLengthField(packetLength);
            hello->setRouterID(ospfPacket->getRouterID());
            hello->setAreaID(ospfPacket->getAreaID());
            hello->setChecksum(ospfPacket->getChecksum());
            hello->setChecksumMode(CHECKSUM_COMPUTED);
            hello->setInstanceID(ospfPacket->getInstanceID());
            hello->setReserved(ospfPacket->getReserved());
            hello->setInterfaceID(stream.readUint32Be());
            hello->setRouterPriority(stream.readByte());
            deserializeOspfOptions(stream, hello->getOptionsForUpdate());
            hello->setHelloInterval(stream.readUint16Be());
            hello->setDeadInterval(stream.readUint16Be());
            hello->setDesignatedRouterID(stream.readIpv4Address());
            hello->setBackupDesignatedRouterID(stream.readIpv4Address());
            int numNeighbors = (B(packetLength) - OSPFV3_HEADER_LENGTH - B(20)).get<B>() / 4;
            if (numNeighbors < 0)
                hello->markIncorrect();
            else
                hello->setNeighborIDArraySize(numNeighbors);
            for (int i = 0; i < numNeighbors; ++i)
                hello->setNeighborID(i, stream.readIpv4Address());
            return hello;
        }
        case DATABASE_DESCRIPTION_PACKET: {
            auto dd = makeShared<Ospfv3DatabaseDescriptionPacket>();
            dd->setChunkLength(B(packetLength));
            dd->setVersion(ospfPacket->getVersion());
            dd->setType(ospfPacket->getType());
            dd->setPacketLengthField(packetLength);
            dd->setRouterID(ospfPacket->getRouterID());
            dd->setAreaID(ospfPacket->getAreaID());
            dd->setChecksum(ospfPacket->getChecksum());
            dd->setChecksumMode(CHECKSUM_COMPUTED);
            dd->setInstanceID(ospfPacket->getInstanceID());
            dd->setReserved(ospfPacket->getReserved());
            dd->setReserved1(stream.readByte());
            deserializeOspfOptions(stream, dd->getOptionsForUpdate());
            dd->setInterfaceMTU(stream.readUint16Be());
            auto& ddo = dd->getDdOptionsForUpdate();
            ddo.reserved = stream.readNBitsToUint64Be(13);
            ddo.iBit = stream.readBit();
            ddo.mBit = stream.readBit();
            ddo.msBit = stream.readBit();
            dd->setSequenceNumber(stream.readUint32Be());
            int numHeaders = (B(packetLength) - OSPFV3_HEADER_LENGTH - OSPFV3_DD_HEADER_LENGTH).get<B>() / OSPFV3_LSA_HEADER_LENGTH.get<B>();
            if (numHeaders < 0)
                dd->markIncorrect();
            else
                dd->setLsaHeadersArraySize(numHeaders);
            for (int i = 0; i < numHeaders; ++i) {
                Ospfv3LsaHeader lsaHeader;
                deserializeLsaHeader(stream, lsaHeader);
                dd->setLsaHeaders(i, lsaHeader);
            }
            return dd;
        }
        case LINKSTATE_REQUEST_PACKET: {
            auto req = makeShared<Ospfv3LinkStateRequestPacket>();
            req->setChunkLength(B(packetLength));
            req->setVersion(ospfPacket->getVersion());
            req->setType(ospfPacket->getType());
            req->setPacketLengthField(packetLength);
            req->setRouterID(ospfPacket->getRouterID());
            req->setAreaID(ospfPacket->getAreaID());
            req->setChecksum(ospfPacket->getChecksum());
            req->setChecksumMode(CHECKSUM_COMPUTED);
            req->setInstanceID(ospfPacket->getInstanceID());
            req->setReserved(ospfPacket->getReserved());
            int numReq = (B(packetLength) - OSPFV3_HEADER_LENGTH).get<B>() / OSPFV3_LSR_LENGTH.get<B>();
            if (numReq < 0)
                req->markIncorrect();
            else
                req->setRequestsArraySize(numReq);
            for (int i = 0; i < numReq; ++i) {
                Ospfv3LsRequest r;
                stream.readUint16Be();
                r.lsaType = stream.readUint16Be();
                r.lsaID = stream.readIpv4Address();
                r.advertisingRouter = stream.readIpv4Address();
                req->setRequests(i, r);
            }
            return req;
        }
        case LINKSTATE_UPDATE_PACKET: {
            auto upd = makeShared<Ospfv3LinkStateUpdatePacket>();
            upd->setChunkLength(B(packetLength));
            upd->setVersion(ospfPacket->getVersion());
            upd->setType(ospfPacket->getType());
            upd->setPacketLengthField(packetLength);
            upd->setRouterID(ospfPacket->getRouterID());
            upd->setAreaID(ospfPacket->getAreaID());
            upd->setChecksum(ospfPacket->getChecksum());
            upd->setChecksumMode(CHECKSUM_COMPUTED);
            upd->setInstanceID(ospfPacket->getInstanceID());
            upd->setReserved(ospfPacket->getReserved());
            uint32_t lsaCount = stream.readUint32Be();
            upd->setLsaCount(lsaCount);
            upd->setLsasArraySize(lsaCount);
            for (uint32_t i = 0; i < lsaCount; ++i) {
                Ospfv3LsaHeader header;
                deserializeLsaHeader(stream, header);
                Ospfv3Lsa *lsa = nullptr;
                switch (header.getLsaType() & 0x1f) {
                    case ROUTER_LSA: {
                        auto *l = new Ospfv3RouterLsa();
                        l->setHeader(header);
                        deserializeRouterLsa(stream, *l);
                        lsa = l;
                        break;
                    }
                    case NETWORK_LSA: {
                        auto *l = new Ospfv3NetworkLsa();
                        l->setHeader(header);
                        deserializeNetworkLsa(stream, *l);
                        lsa = l;
                        break;
                    }
                    case INTER_AREA_PREFIX_LSA: {
                        auto *l = new Ospfv3InterAreaPrefixLsa();
                        l->setHeader(header);
                        deserializeInterAreaPrefixLsa(stream, *l);
                        lsa = l;
                        break;
                    }
                    case LINK_LSA: {
                        auto *l = new Ospfv3LinkLsa();
                        l->setHeader(header);
                        deserializeLinkLsa(stream, *l);
                        lsa = l;
                        break;
                    }
                    case INTRA_AREA_PREFIX_LSA: {
                        auto *l = new Ospfv3IntraAreaPrefixLsa();
                        l->setHeader(header);
                        deserializeIntraAreaPrefixLsa(stream, *l);
                        lsa = l;
                        break;
                    }
                    default:
                        upd->markIncorrect();
                        return upd;
                }
                upd->setLsas(i, lsa);    // @owned: the packet takes ownership
            }
            return upd;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            auto ack = makeShared<Ospfv3LinkStateAcknowledgementPacket>();
            ack->setChunkLength(B(packetLength));
            ack->setVersion(ospfPacket->getVersion());
            ack->setType(ospfPacket->getType());
            ack->setPacketLengthField(packetLength);
            ack->setRouterID(ospfPacket->getRouterID());
            ack->setAreaID(ospfPacket->getAreaID());
            ack->setChecksum(ospfPacket->getChecksum());
            ack->setChecksumMode(CHECKSUM_COMPUTED);
            ack->setInstanceID(ospfPacket->getInstanceID());
            ack->setReserved(ospfPacket->getReserved());
            int numHeaders = (B(packetLength) - OSPFV3_HEADER_LENGTH).get<B>() / OSPFV3_LSA_HEADER_LENGTH.get<B>();
            if (numHeaders < 0)
                ack->markIncorrect();
            else
                ack->setLsaHeadersArraySize(numHeaders);
            for (int i = 0; i < numHeaders; ++i) {
                Ospfv3LsaHeader lsaHeader;
                deserializeLsaHeader(stream, lsaHeader);
                ack->setLsaHeaders(i, lsaHeader);
            }
            return ack;
        }
        default: {
            ospfPacket->markIncorrect();
            return ospfPacket;
        }
    }
}

} // namespace ospfv3
} // namespace inet
