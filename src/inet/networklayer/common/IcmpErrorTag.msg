//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;
import inet.networklayer.ipv4.IcmpHeader;
import inet.networklayer.icmpv6.Icmpv6Header;

namespace inet;

//
// ICMPv4 error indication. Attached by the Icmp module when an ICMPv4 error
// message is received. Each layer pops its own header from the original
// packet and converts it to tags before forwarding the indication upward.
//
class Icmpv4ErrorInd extends TagBase
{
    IcmpType type;          // ICMPv4 type (e.g. ICMP_DESTINATION_UNREACHABLE)
    int code;               // ICMPv4 code (e.g. ICMP_DU_FRAGMENTATION_NEEDED)
    int mtu = -1;           // only relevant for Fragmentation Needed
    Packet *originalPacket @owned;  // progressively unwrapped quoted packet
}

//
// ICMPv6 error indication. Attached by the Icmpv6 module when an ICMPv6 error
// message is received. Each layer pops its own header from the original
// packet and converts it to tags before forwarding the indication upward.
//
class Icmpv6ErrorInd extends TagBase
{
    Icmpv6Type type;        // ICMPv6 type (e.g. ICMPv6_PACKET_TOO_BIG)
    int code;               // ICMPv6 code
    int mtu = -1;           // only relevant for Packet Too Big
    Packet *originalPacket @owned;  // progressively unwrapped quoted packet
}
