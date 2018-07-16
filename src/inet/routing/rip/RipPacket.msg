//
// Copyright (C) 2013 Opensim Ltd.
// Author: Tamas Borbely
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
