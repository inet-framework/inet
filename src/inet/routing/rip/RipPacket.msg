//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet;

cplusplus {{
const B RIP_HEADER_SIZE = B(4);
const B RIP_RTE_SIZE = B(20);
}}

//
// RIP commands
//
enum RipCommand {
    RIP_REQUEST  = 1;
    RIP_RESPONSE = 2;
}

//
// Address Family constants
//
enum RipAf {
    RIP_AF_NONE = 0;
    RIP_AF_INET = 2;
    // RIP_AF_AUTH = 0xFFFF;
}

//
// RIPv2 route entry
//
// see RFC 2453 3.6 and 4
//
struct RipEntry
{
    @packetData;
    RipAf addressFamilyId; // usually AF_INET (2), but NONE (0) for whole table requests
    unsigned short routeTag;        // 0 for RIPv1
    L3Address address;                // destination address
    int prefixLength;               // prefix length of the destination
    L3Address nextHop;                // next hop address; may be unspecified if the packet is received from the next hop
    unsigned int metric;            // metric of the route (1-15) or infinity (16) if no route to the destination
}

//
// RIPv2 packet
//
// note: Authentication entry is not allowed
//
class RipPacket extends FieldsChunk
{
    RipCommand command;
    // version is fixed: 2
    RipEntry entry[];
}
