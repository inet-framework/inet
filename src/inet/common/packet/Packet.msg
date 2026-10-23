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
    b dataLength @label("data length") @hint("Length of packet data excluding popped parts");
    ChunkTemporarySharedPtr *data @packetData @getter(getData) @hint("Packet data, excluding popped parts");
    ChunkTemporarySharedPtr *dissection @packetData @getter(getDissection) @label("dissected data") @hint("Packet data, excluding popped parts, according to protocol dissectors");
    cObject *tags[] @getter(_getTag) @sizeGetter(getNumTags) @hint("Metadata attached to the whole packet");
    SharingRegionTagSet::TagBaseRegionTag regionTags[] @getter(_getRegionTag) @sizeGetter(getNumRegionTags) @label("region tags") @hint("Metadata attached to bit regions");

    b totalLength @group(content) @label("full length") @hint("Length of packet data including popped parts");
    b frontOffset @group(content) @label("front pop offset") @hint("Front pop position");
    b backOffset @group(content) @label("back pop offset") @hint("Back pop position, measured from the beginning of the packet (where the data part ends)");
    ChunkTemporarySharedPtr *front @packetData @getter(getFront) @group(content) @label("popped front") @hint("Packet data popped from the front");
    ChunkTemporarySharedPtr *back @packetData @getter(getBack) @group(content) @label("popped back") @hint("Packet data popped from the back");
    Chunk *content @packetData @getter(getContent) @group(content) @label("full content") @hint("Complete packet data, including popped parts");
}
