//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// Packet for ~AckingMac. Packet size is configurable
// in the MAC layer.
//
class AckingMacHeader extends FieldsChunk
{
    MacAddress src;     // source address
    MacAddress dest;    // destination address
    int srcModuleId;    // technical data, uses instead of sending back an ACK packet
    int networkProtocol = -1;
}

class AckingMacTrailer extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs = 0;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
}
