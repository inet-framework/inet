//
// Copyright (C) 2012 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// IS-IS Protocol Data Units, ported from the ANSA project (ISISMessage.msg,
// author Matej Hrncirik) to the modern INET Chunk packet API.
//
// ANSA modeled every PDU's variable part as a single array of raw
// {type,length,uint8_t* value} TLVs parsed by hand. Here each PDU instead
// carries the TLVs it actually uses as typed, named fields/record arrays
// (the idiom used by INET's EIGRP model), which is clearer and
// serializer-friendly. The Level-1/Level-2 PDU variants differ only in the
// pduType value (and destination multicast MAC), so they are represented by a
// single class each, with pduType set when the PDU is built.
//

import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.contract.ipv6.Ipv6Address;

cplusplus {{
#include "inet/routing/isis/IsisCommon.h"
}}

namespace inet::isis;

// IS-IS identifier value types (see IsisCommon.h), embedded by value.
class SystemId { @existingClass; @opaque; @toString(.str()); }
class AreaId { @existingClass; @opaque; @toString(.str()); }
class PseudonodeId { @existingClass; @opaque; @toString(.str()); }
class LspId { @existingClass; @opaque; @toString(.str()); }

//
// IS-IS PDU types (the "PDU Type" field of the common header).
//
enum IsisPduType
{
    LAN_L1_HELLO = 15;
    LAN_L2_HELLO = 16;
    PTP_HELLO = 17;
    L1_LSP = 18;
    L2_LSP = 20;
    L1_CSNP = 24;
    L2_CSNP = 25;
    L1_PSNP = 26;
    L2_PSNP = 27;
}

//
// Circuit / IS levels.
//
enum IsisCircuitType
{
    L1_TYPE = 1;   // Level 1 only
    L2_TYPE = 2;   // Level 2 only
    L1L2_TYPE = 3; // both Level 1 and Level 2
}

//
// RFC 5303 point-to-point three-way handshake adjacency state.
//
enum IsisPtpAdjacencyState
{
    PTP_ADJ_UP = 0;
    PTP_ADJ_INITIALIZING = 1;
    PTP_ADJ_DOWN = 2;
}

// Network layer protocol identifiers (NLPID) for the Protocols Supported TLV.
enum IsisNlpid
{
    NLPID_CLNP = 0x81;
    NLPID_IPV4 = 0xCC;
    NLPID_IPV6 = 0x8E;
}

//
// Common 8-byte IS-IS PDU header (ISO/IEC 10589). Do not instantiate directly;
// use one of the derived PDU types.
//
class IsisPdu extends FieldsChunk
{
    chunkLength = B(8);
    uint8_t irpd = 0x83;            // Intradomain Routing Protocol Discriminator (IS-IS)
    uint8_t lengthIndicator = 8;    // length of the PDU header in bytes
    uint8_t version = 1;            // version / protocol ID extension
    uint8_t idLength = 0;           // system ID length; 0 means the default 6 bytes
    uint8_t pduType @enum(IsisPduType);
    uint8_t version2 = 1;
    uint8_t reserved = 0;
    uint8_t maxAreas = 1;           // number of areas permitted; 0 means the default 3
}

//
// Common base for the IS-IS Hello (IIH) PDUs. Carries the TLVs present in every
// Hello: Area Addresses (TLV 1), Protocols Supported (TLV 129) and IP Interface
// Address (TLV 132).
//
class IsisHelloPacket extends IsisPdu
{
    uint8_t circuitType @enum(IsisCircuitType) = L1L2_TYPE;
    SystemId sourceId;
    uint16_t holdTime = 40;         // adjacency hold time in seconds
    AreaId areaAddresses[];
    uint8_t protocolsSupported[];   // NLPIDs, see IsisNlpid
    Ipv4Address ipInterfaceAddresses[];      // TLV 132
    Ipv6Address ipv6InterfaceAddresses[];    // TLV 232 (RFC 5308), link-local next hops
}

//
// LAN Hello (Level 1 or Level 2), sent on broadcast circuits. Adds the DIS
// election priority, the LAN ID, and the IS Neighbours TLV (TLV 6, the SNPA/MAC
// addresses of the neighbours heard on this LAN).
//
class IsisLanHelloPacket extends IsisHelloPacket
{
    uint8_t priority = 64;          // DIS election priority (0..127)
    PseudonodeId lanId;             // DIS system ID + pseudonode number
    MacAddress lanNeighbours[];     // TLV 6 IS Neighbours (heard SNPAs)
}

//
// Point-to-point Hello (RFC 5303), sent on point-to-point circuits. Adds the
// local circuit ID and the optional three-way handshake adjacency state TLV
// (TLV 240).
//
class IsisPtpHelloPacket extends IsisHelloPacket
{
    pduType = PTP_HELLO;
    uint8_t localCircuitId = 1;
    bool threeWayHandshake = false; // whether the adjacency-state TLV below is present
    uint8_t adjacencyState @enum(IsisPtpAdjacencyState) = PTP_ADJ_DOWN;
    uint32_t extendedLocalCircuitId = 0;
    SystemId neighbourSystemId;
    uint32_t neighbourExtendedLocalCircuitId = 0;
}

//
// One entry of the IS Neighbours TLV (TLV 2) of a Link State PDU: a neighbouring
// IS or pseudonode reachable from the originator, with the metric of the link.
// Wide (RFC 5305 style) metric.
//
struct IsisIsReachability
{
    @packetData;
    uint32_t metric = 10;
    PseudonodeId neighbourId;
}

//
// One entry of an IP Reachability TLV (TLV 128 internal / TLV 130 external) of a
// Link State PDU: an IPv4 prefix reachable from the originator, used by
// Integrated IS-IS (RFC 1195) to install IP routes.
//
struct IsisIpReachability
{
    @packetData;
    uint32_t metric = 10;
    Ipv4Address address;
    Ipv4Address mask;
}

//
// One entry of the IPv6 Reachability TLV (TLV 236, RFC 5308) of a Link State
// PDU: an IPv6 prefix reachable from the originator, used by Integrated IS-IS
// to install IPv6 routes.
//
struct IsisIpv6Reachability
{
    @packetData;
    uint32_t metric = 10;
    Ipv6Address address;
    uint8_t prefixLength;
}

//
// One entry of the LSP Entries TLV (TLV 9), describing a single LSP by ID,
// sequence number, remaining lifetime and checksum. Carried in CSNPs (to
// advertise the database contents) and PSNPs (to request or acknowledge LSPs).
//
struct IsisLspEntry
{
    @packetData;
    uint16_t remainingLifetime;
    LspId lspId;
    uint32_t sequenceNumber;
    uint16_t checksum;
}

//
// Link State PDU (Level 1 or Level 2). Carries the originator's area addresses,
// supported protocols, IS-neighbour reachability (for the SPF graph) and IP
// reachability (for Integrated IS-IS route installation).
//
class IsisLspPacket extends IsisPdu
{
    uint16_t remainingLifetime = 1200;
    LspId lspId;
    uint32_t sequenceNumber = 1;
    uint16_t checksum = 0;
    // P (1b) | ATT (4b) | OL (1b) | IS Type (2b); default = Level-1 IS.
    uint8_t lspFlags = 0x01;
    AreaId areaAddresses[];
    uint8_t protocolsSupported[];                  // NLPIDs, see IsisNlpid
    IsisIsReachability isReachabilities[];         // TLV 2
    IsisIpReachability ipInternalReachabilities[]; // TLV 128
    IsisIpReachability ipExternalReachabilities[]; // TLV 130
    IsisIpv6Reachability ipv6Reachabilities[];     // TLV 236 (RFC 5308)
}

//
// Complete Sequence Numbers PDU (Level 1 or Level 2): advertises, over the range
// [startLspId, endLspId], the sender's full set of known LSPs as LSP entries.
//
class IsisCsnpPacket extends IsisPdu
{
    PseudonodeId sourceId;
    LspId startLspId;
    LspId endLspId;
    IsisLspEntry lspEntries[];   // TLV 9
}

//
// Partial Sequence Numbers PDU (Level 1 or Level 2): requests or acknowledges a
// partial set of LSPs, listed as LSP entries.
//
class IsisPsnpPacket extends IsisPdu
{
    PseudonodeId sourceId;
    IsisLspEntry lspEntries[];   // TLV 9
}
