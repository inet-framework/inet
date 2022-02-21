//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.common.CrcMode;

namespace inet;

class ApskPhyHeader extends FieldsChunk
{
    chunkLength = B(6);
    b headerLengthField;
    b payloadLengthField;
    uint16_t crc = 0;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    const Protocol *payloadProtocol; // specifies a protocol (internal class with an artificial identifier)
}
