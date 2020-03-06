//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.packet.chunk.Chunk;

namespace inet;

class ApskPhyHeader extends FieldsChunk
{
    chunkLength = B(4);
    b headerLengthField;
    b payloadLengthField;
    const Protocol *payloadProtocol; // specifies a protocol (internal class with an artificial identifier)
}
