//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/EncryptedChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

Register_Class(EncryptedChunk);

EncryptedChunk::EncryptedChunk() :
    Chunk(),
    chunk(nullptr)
{
}

EncryptedChunk::EncryptedChunk(const Ptr<Chunk>& chunk, b length) :
    Chunk(),
    chunk(chunk),
    length(length)
{
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
}

void EncryptedChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->packObject(chunk.get());
}

void EncryptedChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    chunk = check_and_cast<Chunk *>(buffer->unpackObject())->shared_from_this();
}

void EncryptedChunk::forEachChild(cVisitor *v)
{
    Chunk::forEachChild(v);
    v->visit(const_cast<Chunk *>(chunk.get()));
}

bool EncryptedChunk::containsSameData(const Chunk& other) const
{
    if (&other == this)
        return true;
    else if (!Chunk::containsSameData(other))
        return false;
    else {
        auto otherSlice = static_cast<const EncryptedChunk *>(&other);
        return chunk->containsSameData(*otherSlice->chunk.get());
    }
}

const Ptr<Chunk> EncryptedChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    b chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(b(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == b(0) || (iterator.getPosition() == chunkLength && length < b(0))) {
        if (predicate == nullptr || predicate(nullptr))
            return EmptyChunk::getEmptyChunk(flags);
    }
    // 2. peeking the whole part returns this chunk
    if (iterator.getPosition() == b(0) && (-length >= chunkLength || length == chunkLength)) {
        auto result = const_cast<EncryptedChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a SliceChunk
    if (converter == nullptr)
        return peekConverted<SliceChunk>(iterator, length, flags);
    // 4. peeking with conversion
    return converter(const_cast<EncryptedChunk *>(this)->shared_from_this(), iterator, length, flags);
}

void EncryptedChunk::setLength(b length)
{
    handleChange();
    this->length = length;
}

std::ostream& EncryptedChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(chunk, printFieldToString(chunk.get(), level + 1, evFlags));
    return stream;
}

} // namespace

