//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.EtherType;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// This class represents the source and destination address fields of an IEEE
// 802.3 Ethernet MAC header as defined in the sections 3.2.3 Address fields,
// 3.2.4 Destination Address field, 3.2.5 Source Address field of the IEEE Std
// 802.3-2018, IEEE Standard for Ethernet.
//
//  0                   1                   2                   3                   4
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
// +-----------------------------------------------------------------------------------------------+
// |                                      Destination Address                                      |
// +-----------------------------------------------------------------------------------------------+
// |                                        Source Address                                         |
// +-------------------------------+---------------------------------------------------------------+
//
class EthernetMacAddressFields extends FieldsChunk
{
    chunkLength = B(12); // B(6) + B(6)
    // TODO rename to destinationAddress
    MacAddress dest; // The Destination Address field specifies the station(s) for which the MAC frame is intended. It may be an individual or multicast (including broadcast) address.
    // TODO rename to sourceAddress
    MacAddress src; // The Source Address field specifies the station sending the MAC frame.
}

//
// This class represents the EtherType field of an IEEE 802.3 Ethernet MAC header
// as defined in the section 3.2.6 Length/Type field of the IEEE Std 802.3-2018,
// IEEE Standard for Ethernet.
//
//  0                   1
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// +-------------------------------+
// |        EtherType/Length       |
// +-------------------------------+
//
class EthernetTypeOrLengthField extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t typeOrLength; // This two-octet field takes one of two meanings, depending on its numeric value. For numerical evaluation,
                           // the first octet is the most significant octet of this field.
                           // a) If the value of this field is less than or equal to 1500 decimal (05DC hexadecimal), then the Length/
                           //    Type field indicates the number of MAC client data octets contained in the subsequent MAC Client
                           //    Data field of the basic frame (Length interpretation).
                           // b) If the value of this field is greater than or equal to 1536 decimal (0600 hexadecimal), then the
                           //    Length/Type field indicates the Ethertype of the MAC client protocol (Type interpretation).
}

//
// This class represents an IEEE 802.3 Ethernet MAC header as defined in the
// section 3.2 Elements of the MAC frame and packet of the IEEE Std 802.3-2018,
// IEEE Standard for Ethernet.
//
//  0                   1                   2                   3                   4
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
// +-----------------------------------------------------------------------------------------------+
// |                                      Destination Address                                      |
// +-----------------------------------------------------------------------------------------------+
// |                                        Source Address                                         |
// +-------------------------------+---------------------------------------------------------------+
// |           EtherType           |
// +-------------------------------+
//
class EthernetMacHeader extends EthernetMacAddressFields
{
    chunkLength = B(14); // B(6) + B(6) + B(2)
    uint16_t typeOrLength; // This two-octet field takes one of two meanings, depending on its numeric value. For numerical evaluation,
                           // the first octet is the most significant octet of this field.
                           // a) If the value of this field is less than or equal to 1500 decimal (05DC hexadecimal), then the Length/
                           //    Type field indicates the number of MAC client data octets contained in the subsequent MAC Client
                           //    Data field of the basic frame (Length interpretation).
                           // b) If the value of this field is greater than or equal to 1536 decimal (0600 hexadecimal), then the
                           //    Length/Type field indicates the Ethertype of the MAC client protocol (Type interpretation).
}

cplusplus {{
    inline bool isEth2Header(const EthernetMacHeader& hdr) { return isEth2Type(hdr.getTypeOrLength()); }
    inline bool isIeee8023Header(const EthernetMacHeader& hdr) { return isIeee8023Length(hdr.getTypeOrLength()); }
}}

//
// This class represents the variable length padding part of an Ethernet frame
// as defined in the section 3.2.8 Pad field of the IEEE Std 802.3-2018, IEEE
// Standard for Ethernet.
//
//  0                   1
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 ...
// +-------------------------------+
// |   Non-zero number of 0 bits   |
// +-------------------------------+
//
class EthernetPadding extends FieldsChunk
{
}

//
// This class represents an Ethernet FCS as defined in the section 3.2.9 Frame
// Check Sequence (FCS) field of the IEEE Std 802.3-2018, IEEE Standard for
// Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------------------------------------------------------+
// |                              FCS                              |
// +---------------------------------------------------------------+
//
class EthernetFcs extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs; // actual value may be omitted depending on the selected fcsMode

    FcsMode fcsMode = FCS_MODE_UNDEFINED; // meta information, not represented directly in the frame
}

//
// This class represents an Ethernet fragment FCS as defined in the section 3.2.9
// Frame Check Sequence (FCS) field of the IEEE Std 802.3-2018, IEEE Standard for
// Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +---------------------------------------------------------------+
// |                              FCS                              |
// +---------------------------------------------------------------+
//
class EthernetFragmentFcs extends EthernetFcs
{
    bool mCrc; // meta information, not represented directly in the frame
}
