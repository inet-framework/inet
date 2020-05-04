//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.ethernet.Ieee8023TypeOrLength;

namespace inet;

class Ieee8021AeHeader extends Ieee8023TypeOrLength
{
    chunkLength = B(8);     // Length without SCI. Set it to B(16) when hasSci is true.
    typeOrLength = 0x88e5;
    uint8_t tciAn;          // @bit(8);
    uint8_t sl;             // @byte(1);
    uint32_t packetNumber;  // @byte(4);
    bool hasSci = false;    // @bit(0); TODO ????
    uint64_t sci;           // @byte(8); //TODO optional field
}
