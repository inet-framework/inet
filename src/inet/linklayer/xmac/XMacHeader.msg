//
// Copyright (C) 2017 Jan Peter Drees
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

// @brief Types of messages (self messages and packets) the node can process
enum XMacTypes {
    // packet types
    XMAC_PREAMBLE = 191;
    XMAC_DATA = 192;
    XMAC_ACK = 193;
    // self message types
    XMAC_RESEND_DATA = 194;
    XMAC_ACK_TIMEOUT = 195;
    XMAC_START_XMAC = 196;
    XMAC_WAKE_UP = 197;
    XMAC_SEND_ACK = 198;
    XMAC_CCA_TIMEOUT = 199;
    XMAC_ACK_TX_OVER = 200;
    XMAC_SEND_PREAMBLE = 201;
    XMAC_STOP_PREAMBLES = 202;
    XMAC_DATA_TX_OVER = 203;
    XMAC_DATA_TIMEOUT = 204;
    SWITCH_PREAMBLE_PHASE = 205;
    DELAY_FOR_ACK_WITHIN_REMOTE_RX = 206;
    XMAC_SWITCHING_FINISHED = 207;
};

//
// Base class for ~XMac headers.
//
class XMacHeaderBase extends FieldsChunk
{
    MacAddress srcAddr;
    MacAddress destAddr;
    XMacTypes type;
}

//
// Control Frame for ~XMac. Frame size is configurable
// in the MAC layer.
//
class XMacControlFrame extends XMacHeaderBase
{
}

//
// Dataframe header for ~XMac. Frame size is configurable
// in the MAC layer.
//
class XMacDataFrameHeader extends XMacHeaderBase
{
    long sequenceId; // Sequence Number to detect duplicate messages
    int networkProtocol = -1;
}
