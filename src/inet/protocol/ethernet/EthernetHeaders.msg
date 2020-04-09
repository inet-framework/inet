//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.MacAddress;

namespace inet;

class Ieee8023MacAddresses extends FieldsChunk
{
    chunkLength = B(12);
    MacAddress dest;
    MacAddress src;
}

class Ieee8023TypeOrLength extends FieldsChunk
{
    chunkLength = B(2);
    int typeOrLength;
}

//class Ieee8023MacHeader extends FieldsChunk // Ieee8023MacAddresses, Ieee8023TypeOrLength
//{
//    chunkLength = B(14);
//}
//
//class Ieee8021QHeader extends Ieee8023TypeOrLength
//{
//    chunkLength = B(4);
//    // typeOrLength = 0x8100, 0x8a88;
//    uint8_t pcp;       // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
//                       // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
//    bool de;           // Drop Eligible (DE): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
//    uint16_t vid;      // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
//                       // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
//                       // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
//                       // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
//                       // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
//}
//
//class Ieee8021AeHeader extends Ieee8023TypeOrLength
//{
//    chunkLength = B(8);     // Length without SCI. Set it to B(16) when hasSci is true.
//    typeOrLength = 0x88e5;
//    uint8_t tciAn;          // @bit(8);
//    uint8_t sl;             // @byte(1);
//    uint32_t packetNumber;  // @byte(4);
//    bool hasSci = false;    // @bit(0); TODO ????
//    uint64_t sci;           // @byte(8); //TODO optional field
//}
