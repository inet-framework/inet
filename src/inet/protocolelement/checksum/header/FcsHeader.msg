//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.FcsMode;

namespace inet;

class FcsHeader extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs = 0;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
}
