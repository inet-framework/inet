//
// Copyright (C) 2004 Andras Varga
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


//
// PPP header+trailer length:
//    - Flag(8) + Address(8) + Control(8) + Protocol(16) + FCS(16) = 7 octets.
// (The terminating Flag is not counted, as rfc1331 states that
// only one Flag is required between back-to-back frames.)
//
cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/FieldsChunk.h"
}}

namespace inet;

cplusplus {{
const int PPP_OVERHEAD_BYTES = 7;
}}

class FieldsChunk extends cObject;


//
// PPP frame.
//
// Constant-value header fields are not modelled:
// Flag (=0x7e), address (=0xff), control (=0x03), CRC (bitError())
//
// CRC is modelled only via cMessage's bit error attribute.
class PppHeader extends FieldsChunk
{
    int64_t chunkLength = 5;
    short flag = 0x7e;      //1 byte
    short address = 0xff;   //1 byte
    short control = 0x03;   //1 byte
    int protocol = -1;      //2 byte
}

class PppTrailer extends FieldsChunk
{
    int64_t chunkLength = 2; //FIXME correct value is 3, but old inet was used 2 bytes
    short fcs = 0;      // frame check sequence, 2 byte
    short flag = 0x7e;  // 1 byte, omitted for successive PPP packets
}


