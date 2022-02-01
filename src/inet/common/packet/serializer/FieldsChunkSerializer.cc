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
    if (fieldsChunk->getSerializedBytes() != nullptr) {
        CHUNK_CHECK_USAGE(B(fieldsChunk->getSerializedBytes()->size()) == chunk->getChunkLength(), "serialized length is incorrect: serialized=%ld, chunk=%ld", fieldsChunk->getSerializedBytes()->size(), B(chunk->getChunkLength()).get());
        stream.writeBytes(*fieldsChunk->getSerializedBytes(), offset, length == b(-1) ? chunk->getChunkLength() - offset : length);
    }
    else if (offset == b(0) && (length == b(-1) || length == chunk->getChunkLength())) {
        auto startPosition = stream.getLength();
        serialize(stream, fieldsChunk);
        auto endPosition = stream.getLength();
        auto serializedLength = endPosition - startPosition;
        ChunkSerializer::totalSerializedLength += serializedLength;
        auto serializedBytes = new std::vector<uint8_t>();
        stream.copyData(*serializedBytes, startPosition, serializedLength);
        fieldsChunk->setSerializedBytes(serializedBytes);
        CHUNK_CHECK_USAGE(B(serializedBytes->size()) == chunk->getChunkLength(), "serialized length is incorrect: serialized=%ld, chunk=%ld", serializedBytes->size(), B(chunk->getChunkLength()).get());
    }
    else {
        MemoryOutputStream chunkStream(fieldsChunk->getChunkLength());
        serialize(chunkStream, fieldsChunk);
        stream.writeBytes(chunkStream.getData(), offset, length == b(-1) ? chunk->getChunkLength() - offset : length);
        ChunkSerializer::totalSerializedLength += chunkStream.getLength();
        auto serializedBytes = new std::vector<uint8_t>();
        chunkStream.copyData(*serializedBytes);
        fieldsChunk->setSerializedBytes(serializedBytes);
        CHUNK_CHECK_USAGE(B(serializedBytes->size()) == chunk->getChunkLength(), "serialized length is incorrect: serialized=%ld, chunk=%ld", serializedBytes->size(), B(chunk->getChunkLength()).get());
    }
}

const Ptr<Chunk> FieldsChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto fieldsChunk = staticPtrCast<FieldsChunk>(deserialize(stream));
    auto endPosition = stream.getPosition();
    auto chunkLength = endPosition - startPosition;
    ChunkSerializer::totalDeserializedLength += chunkLength;
    fieldsChunk->setChunkLength(chunkLength);
    auto serializedBytes = new std::vector<uint8_t>();
    stream.copyData(*serializedBytes, startPosition, chunkLength);
    fieldsChunk->setSerializedBytes(serializedBytes);
    CHUNK_CHECK_USAGE(B(serializedBytes->size()) == fieldsChunk->getChunkLength(), "serialized length is incorrect: serialized=%ld, chunk=%ld", serializedBytes->size(), B(fieldsChunk->getChunkLength()).get());
    return fieldsChunk;
}

} // namespace

