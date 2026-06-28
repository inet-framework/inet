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
    Ipv4Address ipInterfaceAddresses[];
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
