//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

class HopLimitHeader extends FieldsChunk
{
    chunkLength = B(2);
    int hopLimit;
}
