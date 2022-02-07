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
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.EtherType;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.Ieee802Ctrl;
import inet.linklayer.common.MacAddress;
import inet.linklayer.ieee8021q.Ieee8021qHeader;

cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
}}

namespace inet;

enum EthernetControlOpCode
{
    ETHERNET_CONTROL_PAUSE = 1;
}

enum SapCode;

//
// Ethernet MAC header
//
class EthernetMacHeader extends FieldsChunk
{
    chunkLength = B(ETHER_MAC_HEADER_BYTES);
    MacAddress dest;
    MacAddress src;
    Ieee8021qHeader *cTag @owned;
    Ieee8021qHeader *sTag @owned;
    int typeOrLength;
}

cplusplus(EthernetMacHeader) {{
#if (OMNETPP_BUILDNUM < 1530)
  public:
    Ieee8021qHeader *removeCTag() { return dropCTag(); }
    Ieee8021qHeader *removeSTag() { return dropSTag(); }
#endif
}}

cplusplus {{
    inline bool isEth2Header(const EthernetMacHeader& hdr)  { return isEth2Type(hdr.getTypeOrLength()); }  // typeOrLength is type
    inline bool isIeee8023Header(const EthernetMacHeader& hdr)  { return isIeee8023Length(hdr.getTypeOrLength()); }  // typeOrLength is length
}}

//
//
// Ethernet control frames used by the PAUSE protocol
//
class EthernetControlFrame extends FieldsChunk
{
//    EthernetHeader.etherType = ETHERTYPE_FLOW_CONTROL;
    int opCode = -1;
}

//
// Ethernet frame used by the PAUSE protocol
//
class EthernetPauseFrame extends EthernetControlFrame
{
    chunkLength = ETHER_PAUSE_COMMAND_BYTES;
    opCode = ETHERNET_CONTROL_PAUSE;
    int pauseTime; // in 512 bit-time units
}

class EthernetPadding extends FieldsChunk
{
}

class EthernetFcs extends FieldsChunk
{
    chunkLength = ETHER_FCS_BYTES;
    uint32_t fcs;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
}

cplusplus(EthernetFcs) {{
    EthernetFcs(FcsMode _fcsMode) : EthernetFcs() { fcsMode = _fcsMode; }
}}
