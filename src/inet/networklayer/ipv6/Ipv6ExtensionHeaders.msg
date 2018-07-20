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

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.TlvOptions;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.ipv6.Ipv6Header;
import inet.networklayer.contract.ipv6.Ipv6Address;

cplusplus {{
#include "inet/networklayer/ipv6/Ipv6Header.h"
}}

namespace inet;

cplusplus {{
const B IPv6_FRAGMENT_HEADER_LENGTH = B(8);
}}

//
// Ipv6 TLV option types
//
enum Ipv6TlvOptionTypes
{
    IPv6TLVOPTION_NOP1 = 0;
    IPv6TLVOPTION_NOPN = 1;
    IPv6TLVOPTION_TLV_GPSR = 47;
};


//
// Hop-by-Hop Options Header.
// RFC 2460 Section 4.3
// Next Header Value: 0
//
class Ipv6HopByHopOptionsHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_HOP;
    // byteLength = utils::roundUp(2 + tlvOptions.getLength(), 8);
    TlvOptions tlvOptions;
}

//
// Routing Header
// RFC 2460 Section 4.4
// Next Header Value: 43
//
class Ipv6RoutingHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_ROUTING;
    byteLength = B(8);
    //byteLength = 8+16*getAddressArraySize() when routingType == 0
    uint8 routingType;
    uint8 segmentsLeft;
    Ipv6Address address[];
}

//
// Fragment Header
// RFC 2460 Section 4.5
// Next Header Value: 44
//
class Ipv6FragmentHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_FRAGMENT;
    byteLength = IPv6_FRAGMENT_HEADER_LENGTH;
    unsigned short fragmentOffset;
    unsigned int identification;  // datagram ID: identifies which datagram this fragment belongs to.
    bool moreFragments; //Is this the last fragment?
}

//
// Destination Options Header
// RFC 2460 Section 4.6
// Next Header Value: 60
//
class Ipv6DestinationOptionsHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_DEST;
    byteLength = B(8);    // utils::roundUp(2 + tlvOptions.getLength(), 8);
    TlvOptions tlvOptions;
}

//
// Authentication Header
// RFC 2402
// Not Implemented
//
class Ipv6AuthenticationHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_AUTH;
    byteLength = B(8); // FIXME verify
}

//
// Encapsulating Security Payload Header
// RFC 2406
// Not Implemented
//
class Ipv6EncapsulatingSecurityPayloadHeader extends Ipv6ExtensionHeader
{
    extensionType = IP_PROT_IPv6EXT_ESP;
    byteLength = B(8); // FIXME verify
}
