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

cplusplus {{
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
}}

namespace inet;

class SharingRegionTagSet::TagBaseRegionTag extends cObject
{
    @existingClass;
    @descriptor(readonly);
    b startOffset;
    b endOffset;
    b length;
    cObject* tag @getter(_getTag);
}

class Chunk extends cObject
{
    @packetData;
    @existingClass;
    @beforeChange(handleChange());
    @descriptor(readonly);
    int id @packetData(false) @getter("getChunkId");
    bool mutable @packetData(false) @getter("isMutable");
    bool complete @packetData(false) @getter("isComplete");
    bool correct @packetData(false) @getter("isCorrect");
    bool properlyRepresented @packetData(false) @getter("isProperlyRepresented");
    b chunkLength @packetData(false);
    string rawBin[] @label("raw bin ") @getter("getBinDumpLine") @sizeGetter("getBinDumpNumLines");
    string rawHex[] @label("raw hex ") @getter("getHexDumpLine") @sizeGetter("getHexDumpNumLines");
    SharingRegionTagSet::TagBaseRegionTag regionTags[] @packetData(false) @getter(_getTag) @sizeGetter(getNumTags);
}

class BitCountChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    bool data;
}

class BitsChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    bool bit[];
}

class ByteCountChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    uint8_t data;
}

class BytesChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    uint8_t byte[];
}

class SequenceChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    Chunk* chunks[] @getter(getChunk) @sizeGetter(getNumChunks);
}

class SliceChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    b offset;
    b length;
    Chunk* chunk @getter(_getChunk);
}

class cPacketChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    cPacket* _packet @getter(getPacket);
}

class FieldsChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
}

class ChunkTemporarySharedPtr extends cObject
{
    @existingClass;
    @descriptor(readonly);
}
