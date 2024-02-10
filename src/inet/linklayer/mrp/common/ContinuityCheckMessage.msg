// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

class continuityCheckMessage extends FieldsChunk {
    chunkLength = B(38);
    uint8_t mdLevel = 0;
    uint8_t OpCode = 1;
    uint8_t flags = 0x00000001; //RDI 1 bit,, Traffic 1 bit, Reserved 3 bit, CCM interval 3 bit
    uint8_t firstTLVoffset = 0;
    uint32_t sequenceNumber = 0;
    uint16_t endpointIdentifier = 1;
    uint8_t nameFormat = 4;
    uint8_t nameLength = 20;
    string messageName; //max 41 Bytes
    uint16_t shortNameFormat = 3;
    uint8_t shortNameLength = 2;
    uint16_t shortName = 0x0001;
    uint64_t ituInfo1 = 0;
    uint64_t ituInfo2 = 0;
    uint8_t endTlv = 0;  
}

message continuityCheckTimer
{
    int port;
}

message processDelayTimer
{
    int port;
    int field;
}