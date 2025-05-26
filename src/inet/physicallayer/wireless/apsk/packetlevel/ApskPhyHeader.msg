//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.packet.chunk.Chunk;
import inet.common.checksum.ChecksumMode;
import inet.linklayer.common.FcsMode;

namespace inet;

class ApskPhyHeader extends FieldsChunk
{
    chunkLength = B(8);
    b headerLengthField;
    b payloadLengthField;
    uint16_t fcs = 0;
    ChecksumMode fcsMode = FCS_MODE_UNDEFINED;  // should be FcsMode, but OMMeT++ bug #1351 prevents that (fixed in OMNeT++ 6.2)
    const Protocol *payloadProtocol; // specifies a protocol (internal class with an artificial identifier)
}
