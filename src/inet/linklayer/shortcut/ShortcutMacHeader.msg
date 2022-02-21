//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.packet.chunk.Chunk;

namespace inet;

class ShortcutMacHeader extends FieldsChunk
{
    const Protocol *payloadProtocol;
}
