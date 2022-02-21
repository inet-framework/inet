//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.physicallayer.wired.ethernet.EthernetPhyConstants;

namespace inet::physicallayer;

//
// This is a base class for all IEEE 802.3 Ethernet PHY headers defined the IEEE
// Std 802.3-2018, IEEE Standard for Ethernet.
//
class EthernetPhyHeaderBase extends FieldsChunk
{
    chunkLength = ETHERNET_PHY_HEADER_LEN;
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
