//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.packet.chunk.Chunk;

namespace inet;

class UnitDiskPhyHeader extends FieldsChunk
{
    const Protocol *payloadProtocol; // specifies a protocol (internal class with an artificial identifier)
}
