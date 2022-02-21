//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// Determines the packet type.
//
enum EchoProtocolType
{
    ECHO_PROTOCOL_REQUEST = 0;
    ECHO_PROTOCOL_REPLY = 1;
}

//
// Generic network protocol packet for testing network connectivity.
//
class EchoPacket extends FieldsChunk
{
    chunkLength = B(6);
    EchoProtocolType type;
    int identifier; // identifier to aid in matching Echo replies. May be Zero
    int seqNumber; // sequence number to aid in matching Echo replies. May be Zero
}
