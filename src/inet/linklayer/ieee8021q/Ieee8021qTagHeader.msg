//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// This chunk represents an IEEE 802.1Q tag header as defined by the section
// 9.3 Tag format of the IEEE Std 802.1Q-2018, IEEE Standard for Local and
// Metropolitan Area Networks, Bridges and Bridged Networks.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+-----+-+-----------------------+
// |       TPID (e.g. 0x8100)      | PCP |D|        VLAN ID        |
// +-------------------------------+-----+-+-----------------------+
//
class Ieee8021qTagTpidHeader extends FieldsChunk
{
    chunkLength = B(4);
    uint16_t tpid; // Tag Protocol Identifier (TPID): 16-bit field specifying an EtherType value that is used to identify the frame as a tagged frame and to select the correct tag decoding functions.
    uint8_t pcp;   // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
                   // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
                   // Priority order lowest to highest: 1 0 2 3 4 5 6 7
    bool dei;      // Drop Eligible Indicator (DEI): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
    uint16_t vid;  // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
                   // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
                   // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
                   // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
                   // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
}

//
// This chunk represents an IEEE 802.1Q tag header without the TPID field and
// with the following EtherType Protocol Discrimination (EPD) in the packet.
// The reason for this shifted representation is that it's often easier to look
// at a 802.1Q tag this way during processing. The typeOrLength field contains
// the EtherType as defined in the section 9.2 EtherTypes of the IEEE Std 802-2014,
// IEEE Standard for Local and Metropolitan Area Networks: Overview and Architecture.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-----+-+-----------------------+-------------------------------+
// | PCP |D|        VLAN ID        |        EtherType/Length       |
// +-----+-+-----------------------+-------------------------------+
//
class Ieee8021qTagEpdHeader extends FieldsChunk
{
    chunkLength = B(4);
    uint8_t pcp;           // Priority Code Point (PCP): a 3-bit field which refers to the IEEE 802.1p priority. It indicates the frame priority level.
                           // Values are from 0 (best effort) to 7 (highest); 1 represents the lowest priority.
                           // Priority order lowest to highest: 1 0 2 3 4 5 6 7
    bool dei;              // Drop Eligible Indicator (DEI): a 1-bit field. May be used separately or in conjunction with PCP to indicate frames eligible to be dropped in the presence of congestion.
    uint16_t vid;          // VLAN Identifier (VID): a 12-bit field specifying the VLAN to which the frame belongs.
                           // The hexadecimal values of 0x000 and 0xFFF are reserved. All other values may be used as VLAN identifiers, allowing up to 4,094 VLANs.
                           // The reserved value 0x000 indicates that the frame does not belong to any VLAN; in this case,
                           // the 802.1Q tag specifies only a priority and is referred to as a priority tag. On bridges,
                           // VLAN 1 (the default VLAN ID) is often reserved for a management VLAN; this is vendor-specific.
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
