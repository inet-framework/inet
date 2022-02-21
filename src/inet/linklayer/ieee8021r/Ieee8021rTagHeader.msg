//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// This chunk represents an IEEE 802.1R tag header as defined by the section
// 7.8 Tag format of the IEEE Std 802.1CB-2017, IEEE Standard for Local and
// Metropolitan Area Networks, Frame Replication and Elimination for Reliability.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+-------------------------------+
// |          TPID = 0xF1C1        |          Reserved = 0         |
// +-------------------------------+-------------------------------+
// |         SequenceNumber        |
// +-------------------------------+
//
class Ieee8021rTagTpidHeader extends FieldsChunk
{
    chunkLength = B(6);
    uint16_t sequenceNumber = 0;
}

//
// This chunk represents an IEEE 802.1R tag header without the TPID field and
// with the following EtherType Protocol Discrimination (EPD) in the packet.
// The reason for this shifted representation is that it's often easier to look
// at a 802.1Q tag this way during processing. The typeOrLength field contains
// the EtherType as defined in the section 9.2 EtherTypes of the IEEE Std 802-2014,
// IEEE Standard for Local and Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+-------------------------------+
// |          Reserved = 0         |         SequenceNumber        |
// +-------------------------------+-------------------------------+
// |        EtherType/Length       |
// +-------------------------------+
//
class Ieee8021rTagEpdHeader extends FieldsChunk
{
    chunkLength = B(6);
    uint16_t sequenceNumber = 0;
    uint16_t typeOrLength; // This field contains the 16 bits immediately following the 802.1R tag header TCI field:
                           // I. Tag Protocol Identifier (TPID) of the following tag header: 16-bit field specifying an EtherType value that is used to identify the frame as a tagged frame and to select the correct tag decoding functions.
                           // II. This two-octet field takes one of two meanings, depending on its numeric value. For numerical evaluation,
                           //     the first octet is the most significant octet of this field.
                           //     a) If the value of this field is less than or equal to 1500 decimal (05DC hexadecimal), then the Length/
                           //        Type field indicates the number of MAC client data octets contained in the subsequent MAC Client
                           //        Data field of the basic frame (Length interpretation).
                           //     b) If the value of this field is greater than or equal to 1536 decimal (0600 hexadecimal), then the
                           //        Length/Type field indicates the Ethertype of the MAC client protocol (Type interpretation).
}
