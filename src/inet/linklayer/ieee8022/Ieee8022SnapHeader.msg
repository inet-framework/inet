//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.ieee8022.Ieee8022LlcHeader;

namespace inet;

//
// This chunk represents an IEEE 802.2 LLC header SNAP extension as defined by
// the section 9.5 SNAP of the IEEE Std 802-2014, IEEE Standard for Local and
// Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-----------------------------------------------+---------------+
// |                      OUI                      |   PROTOCOLID  |
// +---------------+-------------------------------+---------------+
// |   PROTOCOLID  |
// +---------------+
//
class Ieee8022SnapHeader extends FieldsChunk
{
    chunkLength = B(5);
    int oui = -1;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.), 3 bytes
    int protocolId = -1; // protocol identifier (PID); stores ~EtherType if orgCode is 0, 2 bytes
}


//
// This chunk represents an IEEE 802.2 LLC header with SNAP extension as defined
// by the section 9.2.4 OUI Extended EtherType of the IEEE Std 802-2014, IEEE
// Standard for Local and Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------+---------------+---------------+---------------+
// |      DSAP     |      SSAP     |    CONTROL    |      OUI      |
// +---------------+---------------+---------------+---------------+
// |              OUI              |           PROTOCOLID          |
// +-------------------------------+-------------------------------+
//
class Ieee8022LlcSnapHeader extends Ieee8022LlcHeader
{
    chunkLength = B(8);
    dsap = 0xAA;
    ssap = 0xAA;
    control = 0x03;
    int oui = -1;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.), 3 bytes
    int protocolId = -1; // protocol identifier (PID); stores ~EtherType if orgCode is 0, 2 bytes
}
