//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.MacAddress;

namespace inet;

enum CsmaCaMacHeaderType {
    CSMA_DATA = 1;
    CSMA_ACK = 2;
};

class CsmaCaMacHeader extends FieldsChunk
{
    CsmaCaMacHeaderType type;    // header type (1 byte)
    uint8_t headerLengthField;    // header length in bytes, max value is 255 (1 byte)
    MacAddress transmitterAddress;    // (6 byte)
    MacAddress receiverAddress;    // (6 byte)
}

class CsmaCaMacAckHeader extends CsmaCaMacHeader
{
    chunkLength = B(14);
    type = CSMA_ACK;
}

class CsmaCaMacDataHeader extends CsmaCaMacHeader
{
    chunkLength = B(17);
    type = CSMA_DATA;
    int networkProtocol = -1;    // ethernet protocol ID (2 bytes)
    int priority = -1;    // 802.1d UP (User Priority) (1 byte)
}

class CsmaCaMacTrailer extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs = 0;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
}
