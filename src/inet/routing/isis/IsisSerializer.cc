//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/isis/IsisSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/isis/IsisMessages_m.h"

namespace inet {
namespace isis {

Register_Serializer(IsisLanHelloPacket, IsisLanHelloSerializer);
Register_Serializer(IsisPtpHelloPacket, IsisPtpHelloSerializer);
Register_Serializer(IsisLspPacket, IsisLspSerializer);
Register_Serializer(IsisCsnpPacket, IsisCsnpSerializer);
Register_Serializer(IsisPsnpPacket, IsisPsnpSerializer);

// ---- field helpers --------------------------------------------------------

static void writeHeader(MemoryOutputStream& s, const IsisPdu *pdu)
{
    s.writeUint8(pdu->getIrpd());
    s.writeUint8(pdu->getLengthIndicator());
    s.writeUint8(pdu->getVersion());
    s.writeUint8(pdu->getIdLength());
    s.writeUint8(pdu->getPduType());
    s.writeUint8(pdu->getVersion2());
    s.writeUint8(pdu->getReserved());
    s.writeUint8(pdu->getMaxAreas());
}

static void readHeader(MemoryInputStream& s, IsisPdu *pdu)
{
    pdu->setIrpd(s.readUint8());
    pdu->setLengthIndicator(s.readUint8());
    pdu->setVersion(s.readUint8());
    pdu->setIdLength(s.readUint8());
    pdu->setPduType(s.readUint8());
    pdu->setVersion2(s.readUint8());
    pdu->setReserved(s.readUint8());
    pdu->setMaxAreas(s.readUint8());
}

static void writeSystemId(MemoryOutputStream& s, const SystemId& id) { s.writeUint48Be(id.getSystemId()); }
static SystemId readSystemId(MemoryInputStream& s) { return SystemId(s.readUint48Be()); }

static void writeAreaId(MemoryOutputStream& s, const AreaId& id) { s.writeUint24Be((uint32_t)(id.getAreaId() & 0xFFFFFF)); }
static AreaId readAreaId(MemoryInputStream& s) { return AreaId(s.readUint24Be()); }

static void writePseudonodeId(MemoryOutputStream& s, const PseudonodeId& id)
{
    s.writeUint48Be(id.getSystemId().getSystemId());
    s.writeUint8(id.getCircuitId());
}

static PseudonodeId readPseudonodeId(MemoryInputStream& s)
{
    uint64_t systemId = s.readUint48Be();
    uint8_t circuitId = s.readUint8();
    return PseudonodeId(systemId, circuitId);
}

static void writeLspId(MemoryOutputStream& s, const LspId& id)
{
    s.writeUint48Be(id.getSystemId().getSystemId());
    s.writeUint8(id.getCircuitId());
    s.writeUint8(id.getFragmentId());
}

static LspId readLspId(MemoryInputStream& s)
{
    uint64_t systemId = s.readUint48Be();
    uint8_t circuitId = s.readUint8();
    uint8_t fragmentId = s.readUint8();
    LspId id(PseudonodeId(systemId, circuitId));
    id.setFragmentId(fragmentId);
    return id;
}

static void writeIsReachability(MemoryOutputStream& s, const IsisIsReachability& r)
{
    s.writeUint32Be(r.metric);
    writePseudonodeId(s, r.neighbourId);
}

static IsisIsReachability readIsReachability(MemoryInputStream& s)
{
    IsisIsReachability r;
    r.metric = s.readUint32Be();
    r.neighbourId = readPseudonodeId(s);
    return r;
}

static void writeIpReachability(MemoryOutputStream& s, const IsisIpReachability& r)
{
    s.writeUint32Be(r.metric);
    s.writeIpv4Address(r.address);
    s.writeIpv4Address(r.mask);
}

static IsisIpReachability readIpReachability(MemoryInputStream& s)
{
    IsisIpReachability r;
    r.metric = s.readUint32Be();
    r.address = s.readIpv4Address();
    r.mask = s.readIpv4Address();
    return r;
}

static void writeIpv6Reachability(MemoryOutputStream& s, const IsisIpv6Reachability& r)
{
    s.writeUint32Be(r.metric);
    s.writeIpv6Address(r.address);
    s.writeUint8(r.prefixLength);
}

static IsisIpv6Reachability readIpv6Reachability(MemoryInputStream& s)
{
    IsisIpv6Reachability r;
    r.metric = s.readUint32Be();
    r.address = s.readIpv6Address();
    r.prefixLength = s.readUint8();
    return r;
}

static void writeLspEntry(MemoryOutputStream& s, const IsisLspEntry& e)
{
    s.writeUint16Be(e.remainingLifetime);
    writeLspId(s, e.lspId);
    s.writeUint32Be(e.sequenceNumber);
    s.writeUint16Be(e.checksum);
}

static IsisLspEntry readLspEntry(MemoryInputStream& s)
{
    IsisLspEntry e;
    e.remainingLifetime = s.readUint16Be();
    e.lspId = readLspId(s);
    e.sequenceNumber = s.readUint32Be();
    e.checksum = s.readUint16Be();
    return e;
}

// ---- LAN Hello ------------------------------------------------------------

void IsisLanHelloSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pdu = staticPtrCast<const IsisLanHelloPacket>(chunk);
    writeHeader(stream, pdu.get());
    stream.writeUint8(pdu->getCircuitType());
    writeSystemId(stream, pdu->getSourceId());
    stream.writeUint16Be(pdu->getHoldTime());
    stream.writeUint8(pdu->getPriority());
    writePseudonodeId(stream, pdu->getLanId());
    stream.writeUint8(pdu->getAreaAddressesArraySize());
    for (size_t i = 0; i < pdu->getAreaAddressesArraySize(); i++)
        writeAreaId(stream, pdu->getAreaAddresses(i));
    stream.writeUint8(pdu->getProtocolsSupportedArraySize());
    for (size_t i = 0; i < pdu->getProtocolsSupportedArraySize(); i++)
        stream.writeUint8(pdu->getProtocolsSupported(i));
    stream.writeUint8(pdu->getIpInterfaceAddressesArraySize());
    for (size_t i = 0; i < pdu->getIpInterfaceAddressesArraySize(); i++)
        stream.writeIpv4Address(pdu->getIpInterfaceAddresses(i));
    stream.writeUint8(pdu->getIpv6InterfaceAddressesArraySize());
    for (size_t i = 0; i < pdu->getIpv6InterfaceAddressesArraySize(); i++)
        stream.writeIpv6Address(pdu->getIpv6InterfaceAddresses(i));
    stream.writeUint8(pdu->getLanNeighboursArraySize());
    for (size_t i = 0; i < pdu->getLanNeighboursArraySize(); i++)
        stream.writeMacAddress(pdu->getLanNeighbours(i));
}

const Ptr<Chunk> IsisLanHelloSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pdu = makeShared<IsisLanHelloPacket>();
    readHeader(stream, pdu.get());
    pdu->setCircuitType(stream.readUint8());
    pdu->setSourceId(readSystemId(stream));
    pdu->setHoldTime(stream.readUint16Be());
    pdu->setPriority(stream.readUint8());
    pdu->setLanId(readPseudonodeId(stream));
    uint8_t n;
    n = stream.readUint8();
    pdu->setAreaAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setAreaAddresses(i, readAreaId(stream));
    n = stream.readUint8();
    pdu->setProtocolsSupportedArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setProtocolsSupported(i, stream.readUint8());
    n = stream.readUint8();
    pdu->setIpInterfaceAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpInterfaceAddresses(i, stream.readIpv4Address());
    n = stream.readUint8();
    pdu->setIpv6InterfaceAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpv6InterfaceAddresses(i, stream.readIpv6Address());
    n = stream.readUint8();
    pdu->setLanNeighboursArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setLanNeighbours(i, stream.readMacAddress());
    return pdu;
}

// ---- Point-to-point Hello -------------------------------------------------

void IsisPtpHelloSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pdu = staticPtrCast<const IsisPtpHelloPacket>(chunk);
    writeHeader(stream, pdu.get());
    stream.writeUint8(pdu->getCircuitType());
    writeSystemId(stream, pdu->getSourceId());
    stream.writeUint16Be(pdu->getHoldTime());
    stream.writeUint8(pdu->getLocalCircuitId());
    stream.writeUint8(pdu->getThreeWayHandshake() ? 1 : 0);
    stream.writeUint8(pdu->getAdjacencyState());
    stream.writeUint32Be(pdu->getExtendedLocalCircuitId());
    writeSystemId(stream, pdu->getNeighbourSystemId());
    stream.writeUint32Be(pdu->getNeighbourExtendedLocalCircuitId());
    stream.writeUint8(pdu->getAreaAddressesArraySize());
    for (size_t i = 0; i < pdu->getAreaAddressesArraySize(); i++)
        writeAreaId(stream, pdu->getAreaAddresses(i));
    stream.writeUint8(pdu->getProtocolsSupportedArraySize());
    for (size_t i = 0; i < pdu->getProtocolsSupportedArraySize(); i++)
        stream.writeUint8(pdu->getProtocolsSupported(i));
    stream.writeUint8(pdu->getIpInterfaceAddressesArraySize());
    for (size_t i = 0; i < pdu->getIpInterfaceAddressesArraySize(); i++)
        stream.writeIpv4Address(pdu->getIpInterfaceAddresses(i));
    stream.writeUint8(pdu->getIpv6InterfaceAddressesArraySize());
    for (size_t i = 0; i < pdu->getIpv6InterfaceAddressesArraySize(); i++)
        stream.writeIpv6Address(pdu->getIpv6InterfaceAddresses(i));
}

const Ptr<Chunk> IsisPtpHelloSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pdu = makeShared<IsisPtpHelloPacket>();
    readHeader(stream, pdu.get());
    pdu->setCircuitType(stream.readUint8());
    pdu->setSourceId(readSystemId(stream));
    pdu->setHoldTime(stream.readUint16Be());
    pdu->setLocalCircuitId(stream.readUint8());
    pdu->setThreeWayHandshake(stream.readUint8() != 0);
    pdu->setAdjacencyState(stream.readUint8());
    pdu->setExtendedLocalCircuitId(stream.readUint32Be());
    pdu->setNeighbourSystemId(readSystemId(stream));
    pdu->setNeighbourExtendedLocalCircuitId(stream.readUint32Be());
    uint8_t n;
    n = stream.readUint8();
    pdu->setAreaAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setAreaAddresses(i, readAreaId(stream));
    n = stream.readUint8();
    pdu->setProtocolsSupportedArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setProtocolsSupported(i, stream.readUint8());
    n = stream.readUint8();
    pdu->setIpInterfaceAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpInterfaceAddresses(i, stream.readIpv4Address());
    n = stream.readUint8();
    pdu->setIpv6InterfaceAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpv6InterfaceAddresses(i, stream.readIpv6Address());
    return pdu;
}

// ---- Link State PDU -------------------------------------------------------

void IsisLspSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pdu = staticPtrCast<const IsisLspPacket>(chunk);
    writeHeader(stream, pdu.get());
    stream.writeUint16Be(pdu->getRemainingLifetime());
    writeLspId(stream, pdu->getLspId());
    stream.writeUint32Be(pdu->getSequenceNumber());
    stream.writeUint16Be(pdu->getChecksum());
    stream.writeUint8(pdu->getLspFlags());
    stream.writeUint8(pdu->getAreaAddressesArraySize());
    for (size_t i = 0; i < pdu->getAreaAddressesArraySize(); i++)
        writeAreaId(stream, pdu->getAreaAddresses(i));
    stream.writeUint8(pdu->getProtocolsSupportedArraySize());
    for (size_t i = 0; i < pdu->getProtocolsSupportedArraySize(); i++)
        stream.writeUint8(pdu->getProtocolsSupported(i));
    stream.writeUint8(pdu->getIsReachabilitiesArraySize());
    for (size_t i = 0; i < pdu->getIsReachabilitiesArraySize(); i++)
        writeIsReachability(stream, pdu->getIsReachabilities(i));
    stream.writeUint8(pdu->getIpInternalReachabilitiesArraySize());
    for (size_t i = 0; i < pdu->getIpInternalReachabilitiesArraySize(); i++)
        writeIpReachability(stream, pdu->getIpInternalReachabilities(i));
    stream.writeUint8(pdu->getIpExternalReachabilitiesArraySize());
    for (size_t i = 0; i < pdu->getIpExternalReachabilitiesArraySize(); i++)
        writeIpReachability(stream, pdu->getIpExternalReachabilities(i));
    stream.writeUint8(pdu->getIpv6ReachabilitiesArraySize());
    for (size_t i = 0; i < pdu->getIpv6ReachabilitiesArraySize(); i++)
        writeIpv6Reachability(stream, pdu->getIpv6Reachabilities(i));
}

const Ptr<Chunk> IsisLspSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pdu = makeShared<IsisLspPacket>();
    readHeader(stream, pdu.get());
    pdu->setRemainingLifetime(stream.readUint16Be());
    pdu->setLspId(readLspId(stream));
    pdu->setSequenceNumber(stream.readUint32Be());
    pdu->setChecksum(stream.readUint16Be());
    pdu->setLspFlags(stream.readUint8());
    uint8_t n;
    n = stream.readUint8();
    pdu->setAreaAddressesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setAreaAddresses(i, readAreaId(stream));
    n = stream.readUint8();
    pdu->setProtocolsSupportedArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setProtocolsSupported(i, stream.readUint8());
    n = stream.readUint8();
    pdu->setIsReachabilitiesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIsReachabilities(i, readIsReachability(stream));
    n = stream.readUint8();
    pdu->setIpInternalReachabilitiesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpInternalReachabilities(i, readIpReachability(stream));
    n = stream.readUint8();
    pdu->setIpExternalReachabilitiesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpExternalReachabilities(i, readIpReachability(stream));
    n = stream.readUint8();
    pdu->setIpv6ReachabilitiesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setIpv6Reachabilities(i, readIpv6Reachability(stream));
    return pdu;
}

// ---- Complete Sequence Numbers PDU ----------------------------------------

void IsisCsnpSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pdu = staticPtrCast<const IsisCsnpPacket>(chunk);
    writeHeader(stream, pdu.get());
    writePseudonodeId(stream, pdu->getSourceId());
    writeLspId(stream, pdu->getStartLspId());
    writeLspId(stream, pdu->getEndLspId());
    stream.writeUint8(pdu->getLspEntriesArraySize());
    for (size_t i = 0; i < pdu->getLspEntriesArraySize(); i++)
        writeLspEntry(stream, pdu->getLspEntries(i));
}

const Ptr<Chunk> IsisCsnpSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pdu = makeShared<IsisCsnpPacket>();
    readHeader(stream, pdu.get());
    pdu->setSourceId(readPseudonodeId(stream));
    pdu->setStartLspId(readLspId(stream));
    pdu->setEndLspId(readLspId(stream));
    uint8_t n = stream.readUint8();
    pdu->setLspEntriesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setLspEntries(i, readLspEntry(stream));
    return pdu;
}

// ---- Partial Sequence Numbers PDU -----------------------------------------

void IsisPsnpSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pdu = staticPtrCast<const IsisPsnpPacket>(chunk);
    writeHeader(stream, pdu.get());
    writePseudonodeId(stream, pdu->getSourceId());
    stream.writeUint8(pdu->getLspEntriesArraySize());
    for (size_t i = 0; i < pdu->getLspEntriesArraySize(); i++)
        writeLspEntry(stream, pdu->getLspEntries(i));
}

const Ptr<Chunk> IsisPsnpSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pdu = makeShared<IsisPsnpPacket>();
    readHeader(stream, pdu.get());
    pdu->setSourceId(readPseudonodeId(stream));
    uint8_t n = stream.readUint8();
    pdu->setLspEntriesArraySize(n);
    for (uint8_t i = 0; i < n; i++) pdu->setLspEntries(i, readLspEntry(stream));
    return pdu;
}

} // namespace isis
} // namespace inet
