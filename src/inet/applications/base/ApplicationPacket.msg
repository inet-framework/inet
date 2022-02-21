//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// Generic application packet
//
// TODO rename to ApplicationHeader and remove ByteCountChunk from ApplicationHeaderSerializer
class ApplicationPacket extends FieldsChunk
{
    uint32_t sequenceNumber;
}
