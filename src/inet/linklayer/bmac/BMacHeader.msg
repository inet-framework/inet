//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

// @brief Types of messages (self messages and packets) the node can process
//TODO split to 2 enums: for packets / for self messages
enum BMacType {
    // packet types
    BMAC_PREAMBLE = 191;
    BMAC_DATA = 192;
    BMAC_ACK = 193;
    // self message types
    BMAC_RESEND_DATA = 194;
    BMAC_ACK_TIMEOUT = 195;
    BMAC_START_BMAC = 196;
    BMAC_WAKE_UP = 197;
    BMAC_SEND_ACK = 198;
    BMAC_CCA_TIMEOUT = 199;
    BMAC_ACK_TX_OVER = 200;
    BMAC_SEND_PREAMBLE = 201;
    BMAC_STOP_PREAMBLES = 202;
    BMAC_DATA_TX_OVER = 203;
    BMAC_DATA_TIMEOUT = 204;
};

//
// Base class for ~BMac headers.
//
class BMacHeaderBase extends FieldsChunk
{
    MacAddress srcAddr;
    MacAddress destAddr;
    BMacType   type;
}

//
// Control Frame for ~BMac. Frame size is configurable
// in the MAC layer.
//
class BMacControlFrame extends BMacHeaderBase
{
}

//
// Dataframe header for ~BMac. Frame size is configurable
// in the MAC layer.
//
class BMacDataFrameHeader extends BMacHeaderBase
{
    long sequenceId; // Sequence Number to detect duplicate messages
    int networkProtocol = -1;
}
