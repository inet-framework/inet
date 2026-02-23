//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// Type codes:          // RFC 2461
enum Ipv6NdOptionTypes {
    IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION = 1;
    IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION = 2;
    IPv6ND_PREFIX_INFORMATION = 3;
    IPv6ND_REDIRECTED_HEADER = 4;
    IPv6ND_MTU = 5;
    IPv6ND_ADVERTISEMENT_INTERVAL = 7;    // RFC 3775 Section 7.3
    IPv6ND_HOME_AGENT_INFORMATION_OPTION = 8;    // RFC 3775 Section 7.4
}

class Ipv6NdOption extends cObject
{
    @packetData;
    Ipv6NdOptionTypes type = static_cast<Ipv6NdOptionTypes>(0);    // 1 byte, 0 is invalid
    short optionLength = 0;    // The length of the option in units of 8 octets, 0 is invalid.    // 1 byte
    char paddingBytes[];    // padding bytes on end of option
}

class Ipv6NdSourceTargetLinkLayerAddress extends Ipv6NdOption
{
    optionLength = 1;    // 8 byte ( when using MacAddress)
    MacAddress linkLayerAddress;
}

class Ipv6NdSourceLinkLayerAddress extends Ipv6NdSourceTargetLinkLayerAddress
{
    type = IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION;
}

class Ipv6NdTargetLinkLayerAddress extends Ipv6NdSourceTargetLinkLayerAddress
{
    type = IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION;
}

//
// IPv6ND Prefix Information
// RFC 2461 / RFC 4861 Section 4.6.2
// RFC 3775 Section 7.2 (routerAddressFlag)
//
class Ipv6NdPrefixInformation extends Ipv6NdOption
{
    type = IPv6ND_PREFIX_INFORMATION;
    optionLength = 4;
    unsigned short prefixLength;    // 0..128
    bool onlinkFlag;        //L-bit
    bool autoAddressConfFlag;    //A-bit
    bool routerAddressFlag;        //R-bit: used in case of MIPv6 when the H-bit is set
    unsigned int reserved1 = 0;
    uint32_t validLifetime;    // seconds
    uint32_t preferredLifetime;    // seconds
    uint32_t reserved2 = 0;
    Ipv6Address prefix;
}

class Ipv6NdMtu extends Ipv6NdOption
{
    type = IPv6ND_MTU;
    optionLength = 1;
    uint16_t reserved;
    uint32_t mtu;
}

// MIPv6 New Advertisement Interval Option
// RFC 3775 Section 7.3
//
class Mipv6NdAdvertisementInterval extends Ipv6NdOption
{
    type = IPv6ND_ADVERTISEMENT_INTERVAL;
    optionLength = 1;
    uint16_t reserved;
    uint32_t advertisementInterval;    // milliseconds
}

//MIPv6 Home Agent Information Option
//RFC 3775 Section 7.4
class Mipv6HaInformation extends Ipv6NdOption
{
    type = IPv6ND_HOME_AGENT_INFORMATION_OPTION;
    optionLength = 1;
    uint16_t reserved;
    uint16_t homeAgentPreference;
    uint16_t homeAgentLifetime;    // seconds
}

class Ipv6NdOptions extends cObject {
    @packetData;
    // TLV Options:
    Ipv6NdOption *option[] @owned;  // on end of message
}

cplusplus(Ipv6NdOptions) {{
    virtual const Ipv6NdOption *findOption(Ipv6NdOptionTypes t) const;
    virtual Ipv6NdOption *findOptionForUpdate(Ipv6NdOptionTypes t);
    virtual void insertUniqueOption(size_t k, Ipv6NdOption * option);
    virtual void insertUniqueOption(Ipv6NdOption * option) { insertUniqueOption(option_arraysize, option); }
}}

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
    int code = 0;
}

//
// Router Solicitation Message Format
// RFC 4861 Section 4.1
//
class Ipv6RouterSolicitation extends Ipv6NdMessage
{
    chunkLength = B(8);    // without options
    type = ICMPv6_ROUTER_SOL;
    //Additional ICMP fields
    uint32_t reserved = 0;
    Ipv6NdOptions options;
}

//
// Router Advertisement Message Format
// RFC 2461 Section 4.2
// RFC 3775 Section 7.1, RFC 5175 Section 3. (homeAgentFlag)
//
class Ipv6RouterAdvertisement extends Ipv6NdMessage
{
    chunkLength = B(16);    // without options
    type = ICMPv6_ROUTER_AD;
    //Additional ICMP fields
    unsigned short curHopLimit;

    bool managedAddrConfFlag;    //M-bit
    bool otherStatefulConfFlag;    //O-bit
    bool homeAgentFlag = false;        //H-bit
    uint8_t reserved = 0;

    unsigned short routerLifetime;  // [s] 0 indicates router is not a default router
    unsigned int reachableTime;     // [ms]
    unsigned int retransTimer;      // [ms]
    Ipv6NdOptions options;
}

//
// Neighbour Solicitation Message Format
// RFC 4861 Section 4.3
//
class Ipv6NeighbourSolicitation extends Ipv6NdMessage
{
    chunkLength = B(24);    // without options
    type = ICMPv6_NEIGHBOUR_SOL;
    //Additional ICMP fields
    uint32_t reserved = 0;
    Ipv6Address targetAddress;// MUST NOT be a multicast address.
    Ipv6NdOptions options;
}

//
// Neighbour Advertisement Message Format
// RFC 2461 Section 4.4
//
class Ipv6NeighbourAdvertisement extends Ipv6NdMessage
{
    chunkLength = B(24);    // without options
    type = ICMPv6_NEIGHBOUR_AD;
    //Additional ICMP fields
    bool routerFlag;    //R-flag
    bool solicitedFlag;    //S-flag
    bool overrideFlag;    //O-flag
    uint32_t reserved = 0;
    Ipv6Address targetAddress;// MUST NOT be a multicast address.
    Ipv6NdOptions options;
}

//
// Redirect Message Format
// RFC 2461 Section 4.5
//
class Ipv6Redirect extends Ipv6NdMessage
{
    chunkLength = B(40);    // without options
    type = ICMPv6_REDIRECT;
    //Additional ICMP fields
    Ipv6Address targetAddress;
    Ipv6Address destinationAddress;
    Ipv6NdOptions options;
}

class Ipv6NdControlInfo extends cObject
{
    Ipv6Address nextHop;   // next hop address
    int interfaceId = -1; // interface on which the datagram should be sent
    bool fromHL = false;    // packet came from higher layer
}
