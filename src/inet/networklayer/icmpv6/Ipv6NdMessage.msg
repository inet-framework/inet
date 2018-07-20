//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 Andras Varga
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
import inet.linklayer.common.MacAddress;
import inet.networklayer.icmpv6.Icmpv6Header;
import inet.networklayer.contract.ipv6.Ipv6Address;

namespace inet;

// TLB options (type, length, bytes), length must be divisible by eight, max value is 8*255
// Length in bytes:
cplusplus {{
const B IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH = B(8);    // RFC 2461, Section 4.6.1. Source/Target Link-layer Address
const B IPv6ND_PREFIX_INFORMATION_OPTION_LENGTH = B(32);   // RFC 2461, Section 4.6.2. Prefix Information
const B IPv6ND_REDIRECTED_HEADER_OPTION_LENGTH = B(8);     // 8 + redirected packet, RFC 2461, Section 4.6.3. Redirected Header.
                                                           // The original packet truncated to ensure that the size of the redirect message does not exceed 1280 octets.
const B IPv6ND_MTU_OPTION_LENGTH = B(8);                   // RFC 2461, Section 4.6.4. MTU
}}

// TLB options (type, length, bytes), length must be divisible by eight, max value is 8*255
// Type codes:
enum Ipv6NdOptionTypes {
    IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION = 1;          // RFC 2461
    IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION = 2;          // RFC 2461
}
//
// IPv6ND Prefix Information
// RFC 2461 / RFC 4861 Section 4.6.2
// RFC 3775 Section 7.2 (routerAddressFlag)
//
class Ipv6NdPrefixInformation extends cObject
{
    unsigned short prefixLength;
    bool onlinkFlag;        //L-bit
    bool autoAddressConfFlag;    //A-bit
    bool routerAddressFlag;        //R-bit: used in case of MIPv6 when the H-bit is set
    unsigned int validLifetime;    // seconds
    unsigned int preferredLifetime;    // seconds
    Ipv6Address prefix;
}

// MIPv6 New Advertisement Interval Option
// RFC 3775 Section 7.3
//
class Mipv6NdAdvertisementInterval extends cObject
{
    unsigned int advertisementInterval;    // milliseconds
}

//MIPv6 Home Agent Information Option
//RFC 3775 Section 7.4
class Mipv6HaInformation extends cObject
{
    unsigned int homeAgentPreference;
    unsigned int homeAgentLifetime;    // seconds
}

//
// Neighbour Discovery for Ipv6.
// RFC 2461
//
// ICMP fields inherited from ~Icmpv6Header:
//    - Type
//
// ICMP fields not implemented:
//    - Checksum
//    - Reserved
//
class Ipv6NdMessage extends Icmpv6Header
{
    //customize(true);
    int code = 0;
}

//
// Router Solicitation Message Format
// RFC 4861 Section 4.1
//
class Ipv6RouterSolicitation extends Ipv6NdMessage
{
    type = ICMPv6_ROUTER_SOL;
    //Possible Options   //FIXME these fields are TLV options!

    // The link-layer address of the sender, if known.
    // MUST NOT be included if the Source Address is the unspecified address.
    // Otherwise, it SHOULD be included on link layers that have addresses.
    MacAddress sourceLinkLayerAddress;
}

//
// Router Advertisement Message Format
// RFC 2461 Section 4.2
// RFC 3775 Section 7.1, RFC 5175 Section 3. (homeAgentFlag)
//
class Ipv6RouterAdvertisement extends Ipv6NdMessage
{
    type = ICMPv6_ROUTER_AD;
    //Additional ICMP fields
    unsigned short curHopLimit;
    bool managedAddrConfFlag;    //M-bit
    bool otherStatefulConfFlag;    //O-bit
    bool homeAgentFlag = false;        //H-bit
    unsigned short routerLifetime;  // 0 indicates router is not a default router
    unsigned int reachableTime;
    unsigned int retransTimer;

    //Possible Options   //FIXME these fields are TLV options!

    MacAddress sourceLinkLayerAddress;        //source link layer option
    unsigned int MTU;                //MTU option
    Ipv6NdPrefixInformation prefixInformation[];    //Prefix Information Option
    Mipv6NdAdvertisementInterval advInterval;    //New Advertisement Interval Option (MIPv6)
    Mipv6HaInformation haInformation;    //New Home Agent Information Option (MIPv6)
}

//
// Neighbour Solicitation Message Format
// RFC 4861 Section 4.3
//
class Ipv6NeighbourSolicitation extends Ipv6NdMessage
{
    type = ICMPv6_NEIGHBOUR_SOL;
    //Additional ICMP fields
    Ipv6Address targetAddress;// MUST NOT be a multicast address.

    //Possible Options   //FIXME these fields are TLV options!

    // The link-layer address for the sender.
    // MUST NOT be included when the source IP address is the unspecified address.
    // Otherwise, on link layers that have addresses this option MUST be included in multicast
    // solicitations and SHOULD be included in unicast solicitations.
    MacAddress sourceLinkLayerAddress;
}

//
// Neighbour Advertisement Message Format
// RFC 2461 Section 4.4
//
class Ipv6NeighbourAdvertisement extends Ipv6NdMessage
{
    type = ICMPv6_NEIGHBOUR_AD;
    //Additional ICMP fields
    bool routerFlag;    //R-flag
    bool solicitedFlag;    //S-flag
    bool overrideFlag;    //O-flag
    Ipv6Address targetAddress;// MUST NOT be a multicast address.

    //Possible Options   //FIXME these fields are TLV options!

    MacAddress targetLinkLayerAddress;    //Target Link Layer Address option
}

//
// Redirect Message Format
// RFC 2461 Section 4.5
//
class Ipv6Redirect extends Ipv6NdMessage
{
    type = ICMPv6_REDIRECT;
    //Additional ICMP fields
    Ipv6Address targetAddress;
    Ipv6Address destinationAddress;

    //Possible Options   //FIXME these fields are TLV options!

    MacAddress targetLinkLayerAddress;
        //Redirected Header Encapsulated Msg
}

class Ipv6NdControlInfo extends cObject
{
    Ipv6Address nextHop;   // next hop address
    int interfaceId = -1; // interface on which the datagram should be sent
    bool fromHL = false;    // packet came from higher layer
}

