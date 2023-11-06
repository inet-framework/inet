//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

cplusplus {{
const B PPP_HEADER_LENGTH = B(2);
}}

//
// PPP frame format according to RFC 1661.
//
class PppHeader extends FieldsChunk
{
    chunkLength = PPP_HEADER_LENGTH;
    int protocol = -1;      // 2 bytes
}
