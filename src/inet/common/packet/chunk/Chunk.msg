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

cplusplus {{
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
}}

namespace inet;

class Chunk extends cObject
{
    @existingClass(true);
    @descriptor(readonly);
    bool mutable @getter("isMutable");
    bool complete @getter("isComplete");
    bool correct @getter("isCorrect");
    bool properlyRepresented @getter("isProperlyRepresented");
    bit chunkLength;
}

class BitsChunk extends Chunk
{
    @existingClass(true);
    @descriptor(readonly);
    // TODO: data should be displayed as: 010... (multiple lines)
}

class BytesChunk extends Chunk
{
    @existingClass(true);
    @descriptor(readonly);
    // TODO: data should be displayed as: AA 00 FF ... (multiple lines, plus readable text part?)
}

class SequenceChunk extends Chunk
{
    @existingClass(true);
    @descriptor(readonly);
    Chunk* chunks[] @getter(getChunk) @sizeGetter(getNumChunks);
}

class SliceChunk extends Chunk
{
    @existingClass(true);
    @descriptor(readonly);
    bit offset;
    bit length;
    Chunk* chunk @getter(_getChunk);
}

class cPacketChunk extends Chunk
{
    @existingClass(true);
    @descriptor(readonly);
    cPacket* _packet @getter(getPacket) @getval(.getPacket());
}
