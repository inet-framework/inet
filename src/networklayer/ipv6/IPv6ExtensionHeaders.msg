//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2001-2004 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

#define IPv6_FRAGMENT_HEADER_LENGTH 8
}}

namespace inet;

class noncobject IPv6Address;

class noncobject IPv6ExtensionHeader;

//
// Hop-by-Hop Options Header.
// RFC 2460 Section 4.3
// Next Header Value: 0
//
class IPv6HopByHopOptionsHeader extends IPv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_HOP;
    byteLength = 8; // FIXME verify
}

//
// Routing Header
// RFC 2460 Section 4.4
// Next Header Value: 43
//
class IPv6RoutingHeader extends IPv6ExtensionHeader
{
    @customize(true);
    extensionType = IP_PROT_IPv6EXT_ROUTING;
    byteLength = 8;
    //byteLength = 8+16*getAddressArraySize() when routingType == 0
    uint8 routingType;
    uint8 segmentsLeft;
    IPv6Address address[];
}

//
// Fragment Header
// RFC 2460 Section 4.5
// Next Header Value: 44
//
class IPv6FragmentHeader extends IPv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_FRAGMENT;
    byteLength = IPv6_FRAGMENT_HEADER_LENGTH; // FIXME verify
    unsigned short fragmentOffset;
    unsigned int identification;  // datagram ID: identifies which datagram this fragment belongs to.
    bool moreFragments; //Is this the last fragment?
}

//
// Destination Options Header
// RFC 2460 Section 4.6
// Next Header Value: 60
//
class IPv6DestinationOptionsHeader extends IPv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_DEST;
    byteLength = 8; // FIXME verify
    // byteLength = 20; // FIXME only valid for Home Address Option!
}

//
// Authentication Header
// RFC 2402
// Not Implemented
//
class IPv6AuthenticationHeader extends IPv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_AUTH;
    byteLength = 8; // FIXME verify
}

//
// Encapsulating Security Payload Header
// RFC 2406
// Not Implemented
//
class IPv6EncapsulatingSecurityPayloadHeader extends IPv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_ESP;
    byteLength = 8; // FIXME verify
}
