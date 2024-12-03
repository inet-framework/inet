//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

void FieldsChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto fieldsChunk = staticPtrCast<const FieldsChunk>(chunk);
    auto& chunkStream = fieldsChunk->getSerializedDataForUpdate();
    if (chunkStream.getLength() == b(0)) {
        chunkStream.setCapacity(fieldsChunk->getChunkLength());
        serialize(chunkStream, fieldsChunk);
        ChunkSerializer::totalSerializedLength += chunkStream.getLength();
    }
    CHUNK_CHECK_USAGE(chunkStream.getLength() == fieldsChunk->getChunkLength(), "serialized length is incorrect: serialized=%" PRId64 " bit, chunk=%" PRId64 " bit", chunkStream.getLength().get<b>(), chunk->getChunkLength().get<b>());
    stream.writeData(chunkStream.getData(), offset, length == b(-1) ? fieldsChunk->getChunkLength() - offset : length);
}

const Ptr<Chunk> FieldsChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto fieldsChunk = staticPtrCast<FieldsChunk>(deserialize(stream));
    auto endPosition = stream.getPosition();
    auto chunkLength = endPosition - startPosition;
    ChunkSerializer::totalDeserializedLength += chunkLength;
    fieldsChunk->setChunkLength(chunkLength);
    auto& chunkStream = fieldsChunk->getSerializedDataForUpdate();
    chunkStream.clear();
    chunkStream.writeData(stream.getData(), startPosition, chunkLength);
    CHUNK_CHECK_USAGE(chunkLength == fieldsChunk->getChunkLength(), "serialized length is incorrect: serialized=%" PRId64 " bit, chunk=%" PRId64 " bit.", chunkLength.get<b>(), fieldsChunk->getChunkLength().get<b>());
    return fieldsChunk;
}

} // namespace

