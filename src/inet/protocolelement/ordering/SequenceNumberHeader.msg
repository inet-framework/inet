//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet;

class SequenceNumberHeader extends FieldsChunk
{
    chunkLength = B(2);
    int sequenceNumber;
}
