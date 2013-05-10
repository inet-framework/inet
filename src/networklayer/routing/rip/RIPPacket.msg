//
// Copyright (C) 2013 Opensim Ltd.
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

class noncobject Address;

cplusplus {{
#include "Address.h"

const int MAX_RIP_ENTRIES = 25;
}}

enum RIPCommand {
    RIP_REQUEST  = 1;
    RIP_RESPONSE = 2;
}

enum RIP_AF {
    RIP_AF_NONE = 0;
    RIP_AF_INET = 2;
    RIP_AF_AUTH = 0xFFFF;
}

struct RIPEntry
{
    unsigned short addressFamilyId; // AF_INET (2), 0 if this is a request for the whole routing table (RFC 2453 3.9.1)
    unsigned short routeTag; // 0 for RIPv1
    Address address;    // 0 allowed for the default route
    int prefixLength; // 0 for RIPv1
    Address nextHop;    // 0 for RIPv1
    unsigned int metric; // 1-16
}

//
// TODO
//
packet RIPPacket {
    unsigned char enum(RIPCommand) command;
    // version is fixed: 2
    RIPEntry entry[];
}
