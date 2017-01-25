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


cplusplus {{
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ethernet/Ethernet.h"
}}

namespace inet;

class noncobject MACAddress;

class noncobject MessageId;

enum EtherType;

enum SAPCode;

//
// Abstract base class for various Ethernet traffic types:
// EtherJam, EtherFilledIFG and EtherPhyFrame.
//
packet EtherTraffic
{
}

//
// Represents jam on the Ethernet.
//
packet EtherJam extends EtherTraffic
{
    long abortedPkTreeID = 0;  // this field stores the packetTreeId of the aborted packet
}

//
// Represents a filled inter-frame gap in burst mode.
// Also used for calculating IFG times.
//
packet EtherFilledIFG extends EtherTraffic
{
    bitLength = INTERFRAME_GAP_BITS;
}

//
// Represents an Ethernet PHY frame.
//
// Contains:
//  - preamble: not stored (only contributes to length)
//  - SFD: not stored (only contributes to length)
//  - encapsulated EtherFrame with padding and FCS
//
packet EtherPhyFrame extends EtherTraffic
{
    @customize(true);
    byteLength = PREAMBLE_BYTES + SFD_BYTES;
    bool srcMacFullDuplex;  // meta-info, for detecting model misconfiguration 
}

