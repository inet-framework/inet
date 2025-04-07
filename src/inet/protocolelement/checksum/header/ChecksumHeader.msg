//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.checksum.ChecksumMode;

namespace inet;

class ChecksumHeader extends FieldsChunk
{
    // note: chunk length (1, 2, 4, or 8 bytes) is set programmatically
    uint64_t checksum = 0;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
}
