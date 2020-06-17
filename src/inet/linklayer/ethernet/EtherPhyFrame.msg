//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;
import inet.physicallayer.common.packetlevel.Signal;

cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
}}

namespace inet;

//
// Abstract base class for various Ethernet traffic types:
// EtherJam, EtherFilledIfg and EtherPhyFrame.
//
packet EthernetSignalBase extends physicallayer::Signal
{
    bool srcMacFullDuplex;  // meta-info, for detecting model misconfiguration
    double bitrate;  // meta-info, for detecting model misconfiguration
}

//
// Represents dataframe on the Ethernet.
//
packet EthernetSignal extends EthernetSignalBase
{
}

//
// Represents jam on the Ethernet.
//
packet EthernetJamSignal extends EthernetSignalBase
{
    srcMacFullDuplex = false;
    long abortedPkTreeID = 0;  // this field stores the packetTreeId of the aborted packet
}

//
// Represents a filled inter-frame gap in burst mode.
// Also used for calculating IFG times.
//
packet EthernetFilledIfgSignal extends EthernetSignalBase
{
    srcMacFullDuplex = false;
    bitLength = b(INTERFRAME_GAP_BITS).get();
}

//
// Base class for Ethernet PHY headers.
//
class EthernetPhyHeaderBase extends FieldsChunk
{
    chunkLength = PREAMBLE_BYTES + SFD_BYTES;
}

//
// Represents an Ethernet PHY header.
//
class EthernetPhyHeader extends EthernetPhyHeaderBase
{
}

// http://www.enodenetworks.com/assets/docs/8023br.pdf
enum EthernetPreambleType {
    SFD = 1;         // 0xD5
    SMD_E = SFD;
    SMD_Verify = 2;  // 0x07
    SMD_Respond = 3; // 0x19
    SMD_Sx = 4;      // 0xE6, 0x4C, 0x7F or 0xB3, see smdNumber in EthernetPhyHeader
    SMD_Cx = 5;      // 0x61, 0x52, 0x9E or 0x2A, see smdNumber in EthernetPhyHeader
};

//
// Represents an Ethernet fragment PHY header.
//
class EthernetFragmentPhyHeader extends EthernetPhyHeaderBase
{
    EthernetPreambleType preambleType = SFD;
    uint8_t smdNumber = 0;       // 0-3, see SMD_Sx or SMD_Cx
    uint8_t fragmentNumber = 0;  // 0-3 (real fragment number modulo 4)
}
// preamble bytes: 0x55


