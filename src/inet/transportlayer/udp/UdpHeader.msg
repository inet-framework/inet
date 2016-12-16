//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/packet/FieldsChunk.h"
#include "inet/transportlayer/common/CRC_m.h"
}}

namespace inet;

enum UdpConstants{
    UDP_HEADER_BYTES = 8;
};

class FieldsChunk extends cObject;
enum CrcMode;
//
// Represents an UDP header, to be used with the ~UDP module.
//
class UdpHeader extends FieldsChunk
{
    @customize(true);
    unsigned short srcPort;
    unsigned short destPort;
    int64_t chunkLength @overridegetter = UDP_HEADER_BYTES;
    int totalLengthField = -1;   // UDP header + payload in bytes //TODO fill it in code
    uint16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}

