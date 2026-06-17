//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.checksum.ChecksumMode;
import inet.networklayer.icmpv6.Icmpv6Header;
import inet.networklayer.icmpv6.MldMessage;
import inet.networklayer.contract.ipv6.Ipv6Address;

namespace inet;

//
// Multicast Address Record types carried in an MLDv2 Report (RFC 3810 §5.2.12).
// Same semantics and values as the IGMPv3 Group Record types (RFC 3376), but with
// MLD-specific names so the two enums never collide in a shared translation unit.
//
enum MldGroupRecordType
{
    MLD_MODE_IS_INCLUDE = 1;
    MLD_MODE_IS_EXCLUDE = 2;
    MLD_CHANGE_TO_INCLUDE_MODE = 3;
    MLD_CHANGE_TO_EXCLUDE_MODE = 4;
    MLD_ALLOW_NEW_SOURCES = 5;
    MLD_BLOCK_OLD_SOURCES = 6;
}

//
// MLDv2 Multicast Listener Query (RFC 3810 §5.1, ICMPv6 type 130).
//
// The first 24 bytes coincide with the MLDv1 Query (~MldQuery), so an MLDv1 node can
// parse the common part; an MLDv2 node recognizes the longer form by its length. The
// inherited `maxRespDelay` field holds the 16-bit Maximum Response Code (RFC 3810 §5.1.3:
// 0-32767 in ms, else floating point 1eee mmmm mmmm mmmm).
//
class Mldv2Query extends MldMessage
{
    chunkLength = B(28);             // 24 (MLDv1 Query) + 1 (Resv/S/QRV) + 1 (QQIC) + 2 (Nr of Sources); + sourceList*16
    type = ICMPv6_MLD_QUERY;         // 130
    uint8_t resv = 0;                // 4 bits, reserved
    bool suppressRouterProc = false; // 1 bit, S flag (suppress router-side processing)
    uint8_t robustnessVariable = 0;  // 3 bits, QRV (Querier's Robustness Variable)
    uint8_t queryIntervalCode = 0;   // 8 bits, QQIC (Querier's Query Interval Code)
    Ipv6AddressVector sourceList;    // Nr of Sources (16 bits) + N * 128 bits
}

//
// A Multicast Address Record in an MLDv2 Report (RFC 3810 §5.2.4-5.2.12).
//
class Mldv2MulticastAddressRecord
{
    @packetData;
    int recordType @enum(MldGroupRecordType);
    Ipv6Address groupAddress;
    Ipv6AddressVector sourceList;
    uint32_t auxData[];
}

//
// MLDv2 Multicast Listener Report (RFC 3810 §5.2, ICMPv6 type 143).
//
// `code` carries the first Reserved byte of the ICMPv6 header (always 0); `reserved`
// is the second Reserved field. Records follow.
//
class Mldv2Report extends Icmpv6Header
{
    chunkLength = B(8);              // 4 (type+code+chksum) + 2 (reserved) + 2 (Nr of records); + records
    type = ICMPv6_MLDv2_REPORT;      // 143
    int code = 0;
    uint16_t reserved = 0;
    Mldv2MulticastAddressRecord multicastAddressRecord[];
}
