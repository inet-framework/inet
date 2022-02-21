//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.common.CrcMode;

namespace inet;

class CrcHeader extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}
