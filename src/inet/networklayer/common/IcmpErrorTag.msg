//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;

namespace inet;

//
// This indication specifies the ICMP error data associated with a transport
// layer error indication. Each layer pops its own header from the original
// packet and converts it to tags before forwarding the indication upward.
//
class IcmpErrorInd extends TagBase
{
    int type;               // ICMP type (e.g. ICMP_DESTINATION_UNREACHABLE)
    int code;               // ICMP code (e.g. ICMP_DU_FRAGMENTATION_NEEDED)
    int mtu = -1;           // only relevant for Packet Too Big / Fragmentation Needed
    Packet *originalPacket @owned;  // progressively unwrapped quoted packet
}

//
// Protocol-agnostic error codes for requesting ICMP error generation.
// The network layer maps these to the appropriate ICMP/ICMPv6 type and code.
//
enum IcmpSendErrorCode
{
    ICMP_E_PORT_UNREACHABLE = 0;
}

//
// This request is sent by the transport layer to the network layer to
// request sending an ICMP error message. The transport layer sets the
// networkProtocolHeader (from NetworkProtocolInd) and the originalPacket
// (with transport header on top). The network layer reconstructs the full
// datagram from these and delegates to ICMP.
//
class IcmpSendErrorReq extends TagBase
{
    IcmpSendErrorCode errorCode;
    ChunkPtr networkProtocolHeader;
    Packet *originalPacket @owned;
}
