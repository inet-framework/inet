//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// This chunk represents an IEEE 802.1AE tag header as defined by the section
// 9.3 MAC Security TAG of the IEEE Std 802AE-2018, IEEE Standard for Local and
// metropolitan area networks, Media Access Control (MAC) Security.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+---------------+---------------+
// |              TPID             |     TCI/AN    |       SL      |
// +-------------------------------+---------------+---------------+
// |                               PN                              |
// +---------------------------------------------------------------+
// |                                                               |
// +                              SCI                              +
// |                                                               |
// +---------------------------------------------------------------+
//
class Ieee8021aeTagTpidHeader extends FieldsChunk
{
    chunkLength = B(8); // Length without SCI. Set it to B(16) when hasSci is true.
    uint16_t tpid;      // Tag Protocol Identifier (TPID): 16-bit field specifying an EtherType value that is used to identify the frame as a tagged frame and to select the correct tag decoding functions.
    uint8_t tciAn;
    uint8_t sl;
    uint32_t pn;
    uint64_t sci;
}

//
// This chunk represents an IEEE 802.1AE tag header without the TPID field and
// with the following EtherType Protocol Discrimination (EPD) in the packet.
// The reason for this shifted representation is that it's often easier to look
// at an 802.1AE tag this way during processing. The typeOrLength field contains
// the EtherType as defined in the section 9.2 EtherTypes of the IEEE Std 802-2014,
// IEEE Standard for Local and Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------+---------------+-------------------------------+
// |     TCI/AN    |       SL      |               PN              |
// +---------------+---------------+-------------------------------+
// |              PN               |                               |
// +-------------------------------+                               +
// |                              SCI                              |
// +                               +-------------------------------+
// |                               |        EtherType/Length       |
// +---------------------------------------------------------------+
//
class Ieee8021aeTagEpdHeader extends FieldsChunk
{
    chunkLength = B(8);   // Length without SCI. Set it to B(16) when hasSci is true.
    uint8_t tciAn;
    uint8_t sl;
    uint32_t pn;
    uint64_t sci;
    uint16_t typeOrLength; // This field contains the 16 bits immediately following the 802.1Q tag header TCI field:
                           // I. Tag Protocol Identifier (TPID) of the following tag header: 16-bit field specifying an EtherType value that is used to identify the frame as a tagged frame and to select the correct tag decoding functions.
                           // II. This two-octet field takes one of two meanings, depending on its numeric value. For numerical evaluation,
                           //     the first octet is the most significant octet of this field.
                           //     a) If the value of this field is less than or equal to 1500 decimal (05DC hexadecimal), then the Length/
                           //        Type field indicates the number of MAC client data octets contained in the subsequent MAC Client
                           //        Data field of the basic frame (Length interpretation).
                           //     b) If the value of this field is greater than or equal to 1536 decimal (0600 hexadecimal), then the
                           //        Length/Type field indicates the Ethertype of the MAC client protocol (Type interpretation).
}
