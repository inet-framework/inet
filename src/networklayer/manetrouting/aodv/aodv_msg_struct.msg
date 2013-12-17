//
// Copyright (C) 2011 Zoltan Bojthe
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


cplusplus {{
#include "aodv_msg_struct.h"
}}

class noncobject ManetAddress;

struct AODV_ext
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t type;
    uint8_t length;
    //TODO char *pointer;
}

//
// A generic AODV packet header
//
packet AODV_msg
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t type;
    uint8_t ttl;
    bool prevFix;
    AODV_ext extension[] @getter(getExtensionRef) @sizeGetter(getNumExtension);
}

//
// Extra unreachable destinations
//
struct RERR_udest
{
    @existingClass(true);
    @descriptor(readonly);
    //uint32_t dest_addr;
    ManetAddress dest_addr;
    uint32_t dest_seqno;
}

packet RERR extends AODV_msg
{
    @existingClass(true);
    @descriptor(readonly);
    unsigned short res1;
    unsigned short n;
    unsigned short res2;
    RERR_udest udest[] @getter(getUdestRef);
}

packet RREP extends AODV_msg
{
    @existingClass(true);
    @descriptor(readonly);
    uint16_t res1;
    uint16_t a;
    uint16_t r;
    uint16_t prefix;
    uint16_t res2;
    uint8_t hcnt;
    //uint32_t dest_addr;
    ManetAddress dest_addr;
    uint32_t dest_seqno;
    //u_int32_t orig_addr;
    ManetAddress orig_addr;
    uint32_t lifetime;
    uint32_t cost;
    uint8_t  hopfix;
}

packet RREP_ack extends AODV_msg
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t reserved;
}

packet RREQ extends AODV_msg
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t j;     // Join flag (multicast)
    uint8_t r;     // Repair flag
    uint8_t g;     // Gratuitous RREP flag
    uint8_t d;     // Destination only respond
    uint8_t res1;
    uint8_t res2;
    uint8_t hcnt;
    uint32_t rreq_id;
    //uint32_t dest_addr;
    ManetAddress dest_addr;
    uint32_t dest_seqno;
    //u_int32_t orig_addr;
    ManetAddress orig_addr;
    uint32_t orig_seqno;
    uint32_t cost;
    uint8_t hopfix;
}

