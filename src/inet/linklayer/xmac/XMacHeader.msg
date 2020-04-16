//
// Copyright (C) 2017 Jan Peter Drees
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

