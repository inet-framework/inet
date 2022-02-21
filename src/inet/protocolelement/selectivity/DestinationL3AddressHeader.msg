//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet;

class DestinationL3AddressHeader extends FieldsChunk
{
    chunkLength = B(4);
    L3Address destinationAddress;
}
