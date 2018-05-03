//
// Copyright (C) 2017 OpenSim Ltd.
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


import inet.common.INETDefs;

namespace inet;

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
    ETHERTYPE_8021Q_TAG = 0x8100;
    ETHERTYPE_IPv6 = 0x86DD;
    ETHERTYPE_NEXT_HOP_FORWARDING = 0x86FF;    // not in standard, INET spceific value for @NextHopForwarding
    ETHERTYPE_FLOW_CONTROL = 0x8808;
    ETHERTYPE_8021ad_S_TAG = 0x88A8;
    ETHERTYPE_MPLS_UNICAST = 0x8847;
    ETHERTYPE_MPLS_MULTICAST = 0x8848;
}

cplusplus {{
    inline bool isEth2Type(int typeOrLength)  { return typeOrLength >= 0x600 || typeOrLength == 0; }  // typeOrLength is type
    inline bool isIeee8023Length(int typeOrLength)  { return typeOrLength > 0 && typeOrLength < 0x600; }  // typeOrLength is length
}}

