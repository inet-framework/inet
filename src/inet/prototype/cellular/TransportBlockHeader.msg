//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::cellular;

//
// The header of a dummy transport block. The rest of the transport block is a byte count chunk
// that fills the size the MCS and the allocation give.
//
class TransportBlockHeader extends FieldsChunk
{
    chunkLength = B(8);
    int ueIndex = -1;     // Index of the UE, see IdealControlChannel
    int carrierIndex = -1; // Index of the component carrier at the sender and the receiver
    int mcs = 0;          // Modulation and coding scheme index, see McsTable
}
