//
// Copyright (C) 2001 CTIE, Monash University
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.common.CrcMode;

namespace inet;

cplusplus {{
const B ICMPv6_HEADER_BYTES = B(8);
}}

//
//# TODO consolidate this file's contents with ICMPMessage.msg
//

enum Icmpv6Type
{

    ICMPv6_UNSPECIFIED = 0;
    ICMPv6_DESTINATION_UNREACHABLE = 1;
    ICMPv6_PACKET_TOO_BIG = 2;
    ICMPv6_TIME_EXCEEDED = 3;
    ICMPv6_PARAMETER_PROBLEM = 4;
    ICMPv6_ECHO_REQUEST = 128;
    ICMPv6_ECHO_REPLY = 129;
    ICMPv6_MLD_QUERY = 130;
    ICMPv6_MLD_REPORT = 131;
    ICMPv6_MLD_DONE = 132;
    ICMPv6_ROUTER_SOL = 133;
    ICMPv6_ROUTER_AD = 134;
    ICMPv6_NEIGHBOUR_SOL = 135;
    ICMPv6_NEIGHBOUR_AD = 136;
    ICMPv6_REDIRECT = 137;
    ICMPv6_MLDv2_REPORT = 143;
    ICMPv6_EXPERIMENTAL_MOBILITY = 150;  //Zarrar Yousaf 02.08.07 (FMIPv6 Implementation)
}

//
// ICMPv6 "codes" for type ICMPv6_DESTINATION_UNREACHABLE
//
enum Icmpv6DestUnav
{

    NO_ROUTE_TO_DEST = 0;
    COMM_WITH_DEST_PROHIBITED = 1;
    //2 - NOT ASSIGNED
    ADDRESS_UNREACHABLE = 3;
    PORT_UNREACHABLE = 4;
}

//
// ICMPv6 "codes" for type ICMPv6_TIME_EXCEEDED
//
enum Icmpv6TimeEx
{

    ND_HOP_LIMIT_EXCEEDED = 0;
    ND_FRAGMENT_REASSEMBLY_TIME = 1;
}

//
// ICMPv6 "codes" for type ICMPv6_PARAMETER_PROBLEM
//
enum Icmpv6ParameterProblem
{

    ERROREOUS_HDR_FIELD = 0;
    UNRECOGNIZED_NEXT_HDR_TYPE = 1;
    UNRECOGNIZED_IPV6_OPTION = 2;
}

//
// Represents an ICMPv6 packet.
//
// Notes:
//    1. number of octets excluding the error datagram that is usually appended
//       in optInfo, i.e. the Type|CODE|CHECKSUM|UNUSED/POINTER/MTU/OTHER
//       as defined in RFC2463
//    2. Any ICMP type with MSB set, i.e. >=128 is an Informational ICMP message
//
class Icmpv6Header extends FieldsChunk
{
    chunkLength = ICMPv6_HEADER_BYTES;
    Icmpv6Type type;     // 1 byte
    //int code; // 1 byte //TODO this should be specific to different ICMP types.
    int chksum = 0;     // 2 bytes
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    // 4 bytes 'type' specific values
}

//
// Notes:
//   1. As defined in RFC2463: Section 3
//
class Icmpv6DestUnreachableMsg extends Icmpv6Header
{
    type = ICMPv6_DESTINATION_UNREACHABLE;
    Icmpv6DestUnav code;
    // unused 4 bytes
}

class Icmpv6PacketTooBigMsg extends Icmpv6Header
{
    type = ICMPv6_PACKET_TOO_BIG;
    int code; //Set to 0 by sender and ignored by receiver.
    int MTU; //MTU of next-hop link
}

class Icmpv6TimeExceededMsg extends Icmpv6Header
{
    type = ICMPv6_TIME_EXCEEDED;
    Icmpv6TimeEx code;
}

class Icmpv6ParamProblemMsg extends Icmpv6Header
{
    type = ICMPv6_PARAMETER_PROBLEM;
    Icmpv6ParameterProblem code;
}

//
// ICMPv6 Echo Request packet (RFC2463: Section 4).
// Data is attached through encapsulation (see Icmpv6.cc)
//
class Icmpv6EchoRequestMsg extends Icmpv6Header
{
    type = ICMPv6_ECHO_REQUEST;
    int code = 0; //set to 0.
    int identifier; // identifier to aid in matching Echo replies. May be Zero
    int seqNumber; // sequence number to aid in matching Echo replies. May be Zero
        //Data is attached through encapsulation. See Icmpv6.cc
}

//
// Icmpv6 Echo Reply packet. Data is attached through encapsulation (see Icmpv6.cc)
//
class Icmpv6EchoReplyMsg extends Icmpv6Header
{
    type = ICMPv6_ECHO_REPLY;
    int code = 0; //set to 0.
    int identifier; // identifier to aid in matching Echo replies. May be Zero
    int seqNumber; // sequence number to aid in matching Echo replies. May be Zero
}
