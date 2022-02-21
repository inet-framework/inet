//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

namespace inet;

//
// Represents a TCP/UDP pseudo header, used for crc calculation.
//
class TransportPseudoHeader extends FieldsChunk
{
    L3Address srcAddress;
    L3Address destAddress;
    int networkProtocolId;  // inet specific value, Protocol::getId()
    int protocolId = -1;    // TCP: 6, UDP: 17
    B packetLength = B(-1);
}
