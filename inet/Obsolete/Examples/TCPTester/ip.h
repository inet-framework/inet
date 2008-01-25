//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef _IP_H_
#define _IP_H_

#include <omnetpp.h>


enum IpFlag
{ IP_F_NSET = 0, IP_F_SET = 1 };

enum IpKind
{ IP_DATAGRAM };

//IP header definitions
//Note: The number of bits used in an IP header, do not correspond
//with the variable types used here.
struct IpHeader
{
    //version using version 4 (4 bits)
    int ip_v;

    //internet header length in 32 bit words (4 bits)
    //since no options have been implemented yet,
    //this should be 5
    unsigned int ip_hl;

    //type of service, not implemneted (8 bits)
    //ip_tos;

    //total length (16 bits)
    //length of the datagram measured in octets
    int ip_len;

    //identification (16 bits)
    //used to assemble the fragments of a datagram
    int ip_id;

    //flags
    //bit 0: reserved, must be zero
    //bit 1: don't fragment
    IpFlag df;
    //bit 2: more fragments
    IpFlag mf;

    //fragment offset (13 bits)
    int ip_off;

    //time to live (8 bits)
    int ip_ttl;

    //protocol (8 bits)
    //this should be 6, since only TCP is used (no ICMP etc.)
    int ip_p;

    //header checksum (16 bits)
    //not implemented

    //source address (32 bits)
    int ip_src;

    //destination address (32 bits)
    int ip_dst;

    //options (variable)
    //not implemented
};

#endif // _IP_H
