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

class noncobject L3Address;

cplusplus {{
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

cplusplus {{
const int RIP_HEADER_SIZE = 4;
const int RIP_RTE_SIZE = 20;
}}

//
// RIP commands
//
enum RIPCommand {
    RIP_REQUEST  = 1;
    RIP_RESPONSE = 2;
}

//
// Address Family constants
//
enum RIP_AF {
    RIP_AF_NONE = 0;
    RIP_AF_INET = 2;
    // RIP_AF_AUTH = 0xFFFF;
}

//
// RIPv2 route entry
//
// see RFC 2453 3.6 and 4
//
struct RIPEntry
{
    unsigned short addressFamilyId @enum(RIP_AF); // usually AF_INET (2), but NONE (0) for whole table requests
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
packet RIPPacket {
    unsigned char command @enum(RIPCommand);
    // version is fixed: 2
    RIPEntry entry[];
}
