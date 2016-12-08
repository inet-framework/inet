//
// Copyright (C) 2016 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

class FieldsChunk extends cObject;
class noncobject L3Address;
//
// Represents a TCP/UDP pseudo header, used for crc calculation.
//
class TransportPseudoHeader
{
    L3Address srcAddress;
    L3Address destAddress;
    int reserved = 0;
    int protocolId = -1;    // TCP: 6, UDP: 17
    int64_t packetLength = -1;
}

