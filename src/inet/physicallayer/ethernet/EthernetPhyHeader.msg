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

namespace inet::physicallayer;

//
// This is a base class for all IEEE 802.3 Ethernet PHY headers defined the IEEE
// Std 802.3-2018, IEEE Standard for Ethernet.
//
class EthernetPhyHeaderBase extends FieldsChunk
{
    chunkLength = B(8);
}

//
// This class represents an Ethernet PHY header as defined in the IEEE Std
// 802.3-2018, IEEE Standard for Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------------------------------------------------------+
// |                        Preamble (0x55)                        |
// +                                               +---------------+
// |                                               |      SFD      |
// +-----------------------------------------------+---------------+
//
class EthernetPhyHeader extends EthernetPhyHeaderBase
{
}

//
// http://www.enodenetworks.com/assets/docs/8023br.pdf
//
enum EthernetPhyHeaderType {
    SFD = 1;         // 0xD5
    SMD_E = SFD;
    SMD_Verify = 2;  // 0x07
    SMD_Respond = 3; // 0x19
    SMD_Sx = 4;      // 0xE6, 0x4C, 0x7F or 0xB3, see smdNumber in EthernetPhyHeader
    SMD_Cx = 5;      // 0x61, 0x52, 0x9E or 0x2A, see smdNumber in EthernetPhyHeader
};

//
// This class represents an Ethernet fragment PHY header as defined in the IEEE
// Std 802.3-2018, IEEE Standard for Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------------------------------------------------------+
// |                        Preamble (0x55)                        |
// +                                               +---------------+
// |                                               |     SMD-Sx    |
// +-----------------------------------------------+---------------+
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------------------------------------------------------+
// |                        Preamble (0x55)                        |
// +                               +---------------+---------------+
// |                               |     SMD-Cx    |   FragNumber  |
// +-------------------------------+---------------+---------------+
//
class EthernetFragmentPhyHeader extends EthernetPhyHeaderBase
{
    EthernetPhyHeaderType preambleType = SFD;
    uint8_t smdNumber = 0;       // 0-3, see SMD_Sx or SMD_Cx
    uint8_t fragmentNumber = 0;  // 0-3 (real fragment number modulo 4)
}

