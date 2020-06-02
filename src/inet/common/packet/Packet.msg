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
import inet.common.Units;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/packet/Packet.h"
}}

namespace inet;

class Packet extends cPacket
{
    @existingClass;
    @descriptor(readonly);
    b dataLength;
    b totalLength;
    ChunkTemporarySharedPtr *data @packetData @getter(getData);
    ChunkTemporarySharedPtr *dissection @packetData @getter(getDissection);
    Chunk *content @packetData @getter(getContent);
    cObject *tags[] @getter(_getTag) @sizeGetter(getNumTags);
    SharingRegionTagSet::TagBaseRegionTag regionTags[] @getter(_getRegionTag) @sizeGetter(getNumRegionTags);

    b frontOffset;
    b backOffset;
    ChunkTemporarySharedPtr *front @packetData @getter(getFront);
    ChunkTemporarySharedPtr *back @packetData @getter(getBack);
}
