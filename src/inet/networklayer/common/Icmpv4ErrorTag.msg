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
// Request tag: asks the ICMP module to send an ICMPv4 error message.
// Attached to a Request sent to ICMP via the MessageDispatcher.
//
class Icmpv4SendErrorReq extends TagBase
{
    IcmpType type;          // ICMPv4 type (e.g. ICMP_DESTINATION_UNREACHABLE)
    int code;               // ICMPv4 code (e.g. ICMP_DU_PORT_UNREACHABLE)
    Packet *originalPacket @owned;  // the packet that triggered the error
}
