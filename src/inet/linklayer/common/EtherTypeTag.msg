//
// This program is property of its copyright holder. All rights reserved.
// 

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

//
// Some EtherType values (Ethernet II).
//
enum EtherType
{
    ETHERTYPE_IPv4 = 0x0800;
    ETHERTYPE_ARP = 0x0806;
    ETHERTYPE_TRILL = 0x22F3;
    ETHERTYPE_L2_ISIS = 0x22F4;
    ETHERTYPE_RARP = 0x8035;
    ETHERTYPE_IPv6 = 0x86DD;
    ETHERTYPE_INET_GENERIC = 0x86FF;    // not in standard, INET spceific value for @GenericNetworkProtocol
    ETHERTYPE_MAC_CONTROL = 0x8808;
    ETHERTYPE_MPLS_UNICAST = 0x8847;
    ETHERTYPE_MPLS_MULTICAST = 0x8848;
}

class EtherTypeTagBase extends TagBase
{
    int etherType @enum(EtherType) = -1;
}

class EtherTypeReq extends EtherTypeTagBase
{
}

class EtherTypeInd extends EtherTypeTagBase
{
}
