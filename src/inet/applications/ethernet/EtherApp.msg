//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// Request packet sent by ~EtherAppClient. Contains the number of bytes to be
// sent back as response; if this value is greater than ~1400, the reply
// will consist of several Ethernet frames. See also ~EtherAppResp.
//
class EtherAppReq extends FieldsChunk
{
    long requestId;
    long responseBytes;
}

//
// Packet sent back by ~EtherAppServer in response to an ~EtherAppReq.
//
class EtherAppResp extends FieldsChunk
{
    int requestId;
    int numFrames;
}
