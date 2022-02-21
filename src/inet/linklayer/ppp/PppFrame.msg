//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// PPP header+trailer length:
//    - Flag(8) + Address(8) + Control(8) + Protocol(16) + FCS(16) = 7 octets.
// (The terminating Flag is not counted, as rfc1331 states that
// only one Flag is required between back-to-back frames.)
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

cplusplus {{
const B PPP_HEADER_LENGTH = B(5);
const B PPP_TRAILER_LENGTH = B(2);
}}

//
// PPP frame.
//
// Constant-value header fields are not modelled:
// Flag (=0x7e), address (=0xff), control (=0x03), CRC (bitError())
//
// CRC is modelled only via cMessage's bit error attribute.
class PppHeader extends FieldsChunk
{
    chunkLength = PPP_HEADER_LENGTH;
    short flag = 0x7e;      //1 byte
    short address = 0xff;   //1 byte
    short control = 0x03;   //1 byte
    int protocol = -1;      //2 byte
}

class PppTrailer extends FieldsChunk
{
    chunkLength = PPP_TRAILER_LENGTH; //FIXME correct value is 3, but old inet was used 2 bytes
    short fcs = 0;      // frame check sequence, 2 byte
    short flag = 0x7e;  // 1 byte, omitted for successive PPP packets
}
