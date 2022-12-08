//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
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

class EmptyChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
}

class StreamBufferChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
    simtime_t startTime;
    bps datarate;
    Chunk* streamData @getter(_getStreamData);
}

class EncryptedChunk extends Chunk
{
    @existingClass;
    @descriptor(readonly);
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
