//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/packet/ChunkQueue.h"
}}

namespace inet;

class ChunkQueue extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    b length;
    Chunk* content @getter(getContent);
}
