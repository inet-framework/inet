//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


enum IPProtocolFieldId;
class noncobject IPAddress;

cplusplus {{
#include "IPAddress.h"
#include "IPDatagram_m.h"
}}


//
// Adds IP control information to transport packet.
//
//    required fields:
//            encapsulation of cPacket
//            protocol: use IPProtocolFieldId as defined above
//            destAddr
//    optional fields:
//            srcAddr: default to first interface address
//            codepoint: used in DS_Field (RFC 2474), instead of TOS; default: 0
//            timeToLive: default defined, usually 64
//            dontFragment: default: false
//    not implemented IP parameters:
//            options: IP Options currently not used
//            buffer length
//            buffer pointer
//            Identifier: currently always chosen by IP layer
//            result: currently no result returned
//
// Send/receive IP call: RFC 791 section 3.3
// Excerpt:
//
// SEND (src, dst, prot, TOS, TTL, BufPTR, len, Id, DF, opt => result)
// RECV (BufPTR, prot, => result, src, dst, TOS, len, opt)
//  where:
//    src = source address
//    dst = destination address
//    prot = protocol
//    TOS = type of service
//    TTL = time to live
//    BufPTR = buffer pointer
//    len = length of buffer
//    Id  = Identifier
//    DF = Don't Fragment
//    opt = option data
//    result = response
//      OK = datagram sent/received ok
//      Error = error in arguments or local network error
//
class IPControlInfo
{
    properties:
        omitGetVerb = true;
    fields:
        IPAddress destAddr;
        IPAddress srcAddr;
        short protocol enum(IPProtocolFieldId);
        unsigned char diffServCodePoint;
        short timeToLive;
        bool dontFragment;
};


//
// Control info for routing purposes. OutputPort and nextHopAddr are 
// filled in by IPRouting.
//
// Next hop address is used on a LAN to determine the MAC destination
// address (and it may be used on other multicast networks for similar
// addressing purpose).
//
class IPRoutingDecision
{
    properties:
        omitGetVerb = true;
    fields:
        int inputPort = -1;
        int outputPort = -1;
        IPAddress nextHopAddr;
};


