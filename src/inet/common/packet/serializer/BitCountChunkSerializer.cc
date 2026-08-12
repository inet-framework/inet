//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/BitCountChunkSerializer.h"

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BitCountChunk, BitCountChunkSerializer);

void BitCountChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& bitCountChunk = staticPtrCast<const BitCountChunk>(chunk);
    b serializedLength = length == b(-1) ? bitCountChunk->getChunkLength() - offset : length;
    stream.writeBitRepeatedly(bitCountChunk->getData(), serializedLength.get<b>());
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> BitCountChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitCountChunk = makeShared<BitCountChunk>();
    b length = stream.getRemainingLength();
    if (length > b(0)) {
        // recover the fill value actually on the wire from its first bit, then verify
        // the rest of the stream repeats it
        bool fillValue = stream.readBit();
        bitCountChunk->setData(fillValue);
        if (!stream.readBitRepeatedly(fillValue, (length - b(1)).get<b>()))
            bitCountChunk->markIncorrect();
    }
    bitCountChunk->setLength(b(length));
    ChunkSerializer::totalDeserializedLength += length;
    return bitCountChunk;
}

} // namespace

