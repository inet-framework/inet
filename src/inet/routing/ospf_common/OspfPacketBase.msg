//
// Copyright (C) 2006 Andras Babos and Andras Varga
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
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.transportlayer.common.CrcMode;

namespace inet::ospf;

enum OspfPacketType
{
    HELLO_PACKET = 1;
    DATABASE_DESCRIPTION_PACKET = 2;
    LINKSTATE_REQUEST_PACKET = 3;
    LINKSTATE_UPDATE_PACKET = 4;
    LINKSTATE_ACKNOWLEDGEMENT_PACKET = 5;
}

//
// Represents an OSPF packet header
//
class OspfPacketBase extends FieldsChunk
{
    chunkLength = B(14);
    short version = -1;     // 1 byte
    OspfPacketType type = static_cast<OspfPacketType>(-1);    // 1 byte
    int packetLengthField;  // 2 bytes

    Ipv4Address routerID;   // 4 bytes
    Ipv4Address areaID;     // 4 bytes

    uint16_t crc = 0;       // 2 bytes
    CrcMode crcMode = CRC_MODE_UNDEFINED;
}

