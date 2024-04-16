//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

message ContinuityCheckTimer
{
    int port;
}

message ProcessDelayTimer
{
    int port;
    int field;
}
