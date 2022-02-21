//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/packet/Packet.h"
}}

namespace inet;

class Packet extends cPacket
{
    @existingClass;
    @descriptor(readonly);
    b dataLength;
    b totalLength;
    ChunkTemporarySharedPtr *data @packetData @getter(getData);
    ChunkTemporarySharedPtr *dissection @packetData @getter(getDissection);
    Chunk *content @packetData @getter(getContent);
    cObject *tags[] @getter(_getTag) @sizeGetter(getNumTags);
    SharingRegionTagSet::TagBaseRegionTag regionTags[] @getter(_getRegionTag) @sizeGetter(getNumRegionTags);

    b frontOffset;
    b backOffset;
    ChunkTemporarySharedPtr *front @packetData @getter(getFront);
    ChunkTemporarySharedPtr *back @packetData @getter(getBack);
}
