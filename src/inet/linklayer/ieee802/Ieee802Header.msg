//
// Copyright (C) OpenSim Ltd.
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
#include "inet/common/packet/chunk/FieldsChunk.h"
}}

namespace inet;

class FieldsChunk extends cObject;

class Ieee802LlcHeader extends FieldsChunk
{
    chunkLength = byte(3);
    short dsap = -1;
    short ssap = -1;
    int control = -1;
}

class Ieee802SnapHeader extends Ieee802LlcHeader
{
    chunkLength = byte(8);
    dsap = 0xAA;
    ssap = 0xAA;
    control = 0x03;
    int oui = -1;   // organizationally unique identifier (OUI); 0 for protocols that have an EtherType (ARP, IPv4, IPv6, etc.)
    int protocolId = -1; // protocol identifier (PID); stores ~EtherType if orgCode is 0
}
