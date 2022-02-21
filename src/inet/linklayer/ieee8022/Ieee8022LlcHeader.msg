//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// This chunk represents an IEEE 802.2 LLC header as defined by the ANSI/IEEE
// Std 802.2, 1998, IEEE Standard for Information technology, Telecommunications
// and information exchange between systems, Local and metropolitan area networks.
//
//  0                   1                   2
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
// +---------------+---------------+---------------+
// |      DSAP     |      SSAP     |    CONTROL    |
// +---------------+---------------+---------------+
//
class Ieee8022LlcHeader extends FieldsChunk
{
    chunkLength = B(3);
    short dsap = -1;    // 1 byte
    short ssap = -1;    // 1 byte
    int control = -1;    // 1 or 2 byte, control = byte[0]+256*byte[1], two bytes, when (byte[0] & 3) != 3
}
