//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/BitsChunkSerializer.h"

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(BitsChunk, BitsChunkSerializer);

void BitsChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& bitsChunk = staticPtrCast<const BitsChunk>(chunk);
    b serializedLength = length == b(-1) ? bitsChunk->getChunkLength() - offset : length;
    stream.writeBits(bitsChunk->getBits(), offset, serializedLength);
    ChunkSerializer::totalSerializedLength += serializedLength;
}

const Ptr<Chunk> BitsChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto bitsChunk = makeShared<BitsChunk>();
    b length = stream.getRemainingLength();
    std::vector<bool> chunkBits;
    stream.readBits(chunkBits, length);
    bitsChunk->setBits(chunkBits);
    ChunkSerializer::totalDeserializedLength += length;
    return bitsChunk;
}

} // namespace

