//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2001-2004 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    uint8_t routingType;
    uint8_t segmentsLeft;
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
    uint16_t fragmentOffset;    // @bit(13), divided by 8
    uint8_t reserved = 0;    // @bit(2)
    bool moreFragments; //Is this the last fragment?
    uint32_t identification;  // datagram ID: identifies which datagram this fragment belongs to.
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
