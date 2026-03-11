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
// Protocol-agnostic ICMP error codes used in both directions:
// - Receive: ICMP/ICMPv6 modules map raw type/code to this enum
// - Send: transport layer uses this enum, network layer maps to ICMP/ICMPv6
//
enum IcmpErrorCode
{
    // Destination Unreachable
    ICMP_E_NET_UNREACHABLE = 0;
    ICMP_E_HOST_UNREACHABLE = 1;
    ICMP_E_PROTOCOL_UNREACHABLE = 2;
    ICMP_E_PORT_UNREACHABLE = 3;
    ICMP_E_FRAGMENTATION_NEEDED = 4;    // also Packet Too Big; use mtu field
    ICMP_E_SOURCE_ROUTE_FAILED = 5;
    ICMP_E_ADMIN_PROHIBITED = 6;
    ICMP_E_DESTINATION_UNREACHABLE = 7;  // generic/unmapped dest unreachable

    // Time Exceeded
    ICMP_E_TTL_EXCEEDED = 10;
    ICMP_E_REASSEMBLY_TIMEOUT = 11;

    // Parameter Problem
    ICMP_E_PARAMETER_PROBLEM = 20;
}

//
// This indication specifies the ICMP error data associated with a transport
// layer error indication. Each layer pops its own header from the original
// packet and converts it to tags before forwarding the indication upward.
//
class IcmpErrorInd extends TagBase
{
    IcmpErrorCode errorCode;
    int mtu = -1;           // only relevant for ICMP_E_FRAGMENTATION_NEEDED
    Packet *originalPacket @owned;  // progressively unwrapped quoted packet
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
    IcmpErrorCode errorCode;
    ChunkPtr networkProtocolHeader;
    Packet *originalPacket @owned;
}
