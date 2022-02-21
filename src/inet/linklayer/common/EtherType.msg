//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

//
// Some EtherType values (Ethernet II).
//
enum EtherType
{
    ETHERTYPE_IPv4 = 0x0800;
    ETHERTYPE_ARP = 0x0806;
    ETHERTYPE_INET_CDP = 0x2000;
    ETHERTYPE_SRP = 0x22EA;
    ETHERTYPE_TSN = 0x22F0;
    ETHERTYPE_TRILL = 0x22F3;
    ETHERTYPE_L2_ISIS = 0x22F4;
    ETHERTYPE_INET_FLOODING = 0x36FC;
    ETHERTYPE_RARP = 0x8035;
    ETHERTYPE_8021Q_TAG = 0x8100;
    ETHERTYPE_IPv6 = 0x86DD;
    ETHERTYPE_UNKNOWN = 0x86F0;
    ETHERTYPE_INET_PROBABILISTIC = 0x86FD;
    ETHERTYPE_INET_WISEROUTE = 0x86FE;
    ETHERTYPE_NEXT_HOP_FORWARDING = 0x86FF;    // not in standard, INET spceific value for @NextHopForwarding
    ETHERTYPE_FLOW_CONTROL = 0x8808;
    ETHERTYPE_MPLS_UNICAST = 0x8847;
    ETHERTYPE_MPLS_MULTICAST = 0x8848;
    ETHERTYPE_8021ad_S_TAG = 0x88A8;
    ETHERTYPE_LLDP = 0x88CC;
    ETHERTYPE_IEEE8021AE = 0x88E5;
    ETHERTYPE_GPTP = 0x88F7;
    ETHERTYPE_TTETH = 0x891D;
    ETHERTYPE_IEEE8021_R_TAG = 0xF1C1;
}

cplusplus {{
    inline bool isEth2Type(int typeOrLength)  { return typeOrLength >= 0x600 || typeOrLength == 0; }  // typeOrLength is type
    inline bool isIeee8023Length(int typeOrLength)  { return typeOrLength > 0 && typeOrLength < 0x600; }  // typeOrLength is length
}}
