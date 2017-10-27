//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
}}

namespace inet;


class ChunkQueue extends cNamedObject
{
    @existingClass(true);
    @descriptor(readonly);
    bit length;
    Chunk* contents @getter(getContents);
}

class ChunkBuffer::Region extends void
{
    @existingClass(true);
    @descriptor(readonly);
    bit startOffset;
    bit endOffset;
    Chunk* data @getter(getData);
}

class ChunkBuffer extends cNamedObject
{
    @existingClass(true);
    @descriptor(readonly);
    ChunkBuffer::Region* regions[] @getter(getRegion) @sizeGetter(getNumRegions);
}

packet Packet
{
    @existingClass(true);
    @descriptor(readonly);
    bit totalLength;
    bit headerPoppedLength;
    bit dataLength;
    bit trailerPoppedLength;
    Chunk* contents @packetData @getter(getContents);
}
