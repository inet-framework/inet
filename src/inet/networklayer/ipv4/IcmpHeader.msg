//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.checksum.ChecksumMode;
import inet.networklayer.common.IcmpType;

namespace inet;

//
// ICMP message class
//
// Note: receiver MUST NOT decapsulate the datagram from ICMP error messages,
// because their contents is conceptually truncated. decapsulate() would result
// in a "length became negative" error; use getEncapsulatedMsg() instead.
//
// TODO add originator id and sequence number and create an ICMPControlInfo
class IcmpHeader extends FieldsChunk
{
    chunkLength = B(8);
    IcmpType type;   // 1 byte
    int code = -1;                   // 1 byte
    int chksum = 0;                 // 2 bytes, RFC 1071
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
}

class IcmpEchoRequest extends IcmpHeader
{
    type = ICMP_ECHO_REQUEST;
    code = 0;
    int identifier; // sender module id   // 2 bytes
    int seqNumber; // sequence number           // 2 bytes
}

class IcmpEchoReply extends IcmpHeader
{
    type = ICMP_ECHO_REPLY;
    code = 0;
    int identifier; // sender module id   // 2 bytes
    int seqNumber; // sequence number           // 2 bytes
}

class IcmpPtb extends IcmpHeader
{
    type = ICMP_DESTINATION_UNREACHABLE;
    code = ICMP_DU_FRAGMENTATION_NEEDED;
    uint16_t unused = 0;
    uint16_t mtu = 0;
}
