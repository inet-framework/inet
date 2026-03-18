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
    b dataLength @hint("Length of packet data excluding popped parts");
    b totalLength @hint("Length of packet data including popped parts");
    ChunkTemporarySharedPtr *data @packetData @getter(getData) @hint("Packet data, excluding popped parts");
    ChunkTemporarySharedPtr *dissection @packetData @getter(getDissection) @hint("Complete packet data according to protocol dissectors");
    Chunk *content @packetData @getter(getContent) @hint("Complete packet data, including popped parts");
    cObject *tags[] @getter(_getTag) @sizeGetter(getNumTags) @hint("Metadata attached to the whole packet");
    SharingRegionTagSet::TagBaseRegionTag regionTags[] @getter(_getRegionTag) @sizeGetter(getNumRegionTags) @hint("Metadata attached to bit regions");

    b frontOffset @hint("Front pop position");
    b backOffset @hint("Back pop position");
    ChunkTemporarySharedPtr *front @packetData @getter(getFront) @hint("Packet data popped from the front");
    ChunkTemporarySharedPtr *back @packetData @getter(getBack) @hint("Packet data popped from the back");
}
