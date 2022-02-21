//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

class DestinationMacAddressHeader extends FieldsChunk
{
    chunkLength = B(6);
    MacAddress destinationAddress;
}
