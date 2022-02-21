//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/packet/ReorderBuffer.h"
}}

namespace inet;

class ChunkBuffer::Region
{
    @existingClass;
    @descriptor(readonly);
    b startOffset;
    b endOffset;
    Chunk* data @getter(getData);
}

class ChunkBuffer extends cNamedObject
{
    @existingClass;
    @descriptor(readonly);
    ChunkBuffer::Region* regions[] @getter(getRegion) @sizeGetter(getNumRegions);
}

class ReorderBuffer extends ChunkBuffer
{
    @existingClass;
    @descriptor(readonly);
    b expectedOffset;
}

class ReassemblyBuffer extends ChunkBuffer
{
    @existingClass;
    @descriptor(readonly);
    b expectedLength;
    bool isComplete;
}
