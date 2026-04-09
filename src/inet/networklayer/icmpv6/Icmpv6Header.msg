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
import inet.common.checksum.ChecksumMode;
import inet.networklayer.common.Icmpv6Type;

namespace inet;

cplusplus {{
const B ICMPv6_HEADER_BYTES = B(8);
}}

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
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
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
