//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.ethernet.Ieee8023TypeOrLength;

namespace inet;

class Ieee8021qHeader extends Ieee8023TypeOrLength
{
    chunkLength = B(4);
    uint8_t pcp;       // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
                       // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
    bool de;           // Drop Eligible (DE): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
    uint16_t vid;      // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
                       // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
                       // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
                       // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
                       // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
}

