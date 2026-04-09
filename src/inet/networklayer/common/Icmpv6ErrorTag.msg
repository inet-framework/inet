//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;
import inet.networklayer.common.Icmpv6Type;

namespace inet;

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

//
// Request tag: asks the ICMPv6 module to send an ICMPv6 error message.
// Attached to a Request sent to ICMPv6 via the MessageDispatcher.
//
class Icmpv6SendErrorReq extends TagBase
{
    Icmpv6Type type;        // ICMPv6 type (e.g. ICMPv6_DESTINATION_UNREACHABLE)
    int code;               // ICMPv6 code (e.g. PORT_UNREACHABLE)
    Packet *originalPacket @owned;  // the packet that triggered the error
}
