//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.checksum.ChecksumMode;
import inet.networklayer.icmpv6.Icmpv6Header;
import inet.networklayer.contract.ipv6.Ipv6Address;

namespace inet;

//
// Base class for MLDv1 messages (RFC 2710).
// Carried as ICMPv6 type 130 (Query), 131 (Report), 132 (Done).
// Inherits type, chksum, and checksumMode from Icmpv6Header.
//
class MldMessage extends Icmpv6Header
{
    int code = 0;                       // always 0 for MLD (RFC 2710 §3)
    uint16_t maxRespDelay = 0;          // Maximum Response Delay, ms (16 bits)
    uint16_t reserved = 0;              // reserved (16 bits, must be 0)
    Ipv6Address multicastAddress;       // 128-bit multicast group address
}

//
// MLDv1 Multicast Listener Query (RFC 2710, ICMPv6 type 130).
// General query: multicastAddress = ::
// Group-specific query: multicastAddress = group address
//
class MldQuery extends MldMessage
{
    chunkLength = B(24);        // 4 (type+code/checksum) + 2 (maxRespDelay) + 2 (reserved) + 16 (addr)
    type = ICMPv6_MLD_QUERY;   // 130
}

//
// MLDv1 Multicast Listener Report (RFC 2710, ICMPv6 type 131).
//
class MldReport extends MldMessage
{
    chunkLength = B(24);
    type = ICMPv6_MLD_REPORT;  // 131
}

//
// MLDv1 Multicast Listener Done (RFC 2710, ICMPv6 type 132).
//
class MldDone extends MldMessage
{
    chunkLength = B(24);
    type = ICMPv6_MLD_DONE;    // 132
}
