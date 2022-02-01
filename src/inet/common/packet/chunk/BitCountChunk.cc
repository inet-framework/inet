//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/BitCountChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"

namespace inet {

Register_Class(BitCountChunk);

BitCountChunk::BitCountChunk() :
    Chunk(),
    length(-1),
    data(false)
{
}

BitCountChunk::BitCountChunk(b length, bool data) :
    Chunk(),
    length(length),
    data(data)
{
    CHUNK_CHECK_USAGE(length >= b(0), "length is invalid");
}

void BitCountChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(b(length).get());
    buffer->pack(data);
}

void BitCountChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    int64_t l;
    buffer->unpack(l);
    length = b(l);
    buffer->unpack(data);
}

const Ptr<Chunk> BitCountChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
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
        auto result = const_cast<BitCountChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a BitCountChunk
    if (converter == nullptr) {
        auto result = makeShared<BitCountChunk>(length < b(0) ? std::min(-length, chunkLength - iterator.getPosition()) : length);
        result->regionTags.copyTags(regionTags, iterator.getPosition(), b(0), result->getChunkLength());
        result->markImmutable();
        return result;
    }
    // 4. peeking with conversion
    return converter(const_cast<BitCountChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> BitCountChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    b resultLength = length < b(0) ? std::min(-length, chunkLength - offset) : length;
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= resultLength && resultLength <= chunkLength);
    return makeShared<BitCountChunk>(resultLength);
}

void BitCountChunk::setLength(b length)
{
    CHUNK_CHECK_USAGE(length >= b(0), "length is invalid");
    handleChange();
    this->length = length;
}

void BitCountChunk::setData(bool data)
{
    handleChange();
    this->data = data;
}

bool BitCountChunk::containsSameData(const Chunk& other) const
{
    return &other == this || (Chunk::containsSameData(other) && data == static_cast<const BitCountChunk *>(&other)->data);
}

bool BitCountChunk::canInsertAtFront(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BITCOUNT;
}

bool BitCountChunk::canInsertAtBack(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BITCOUNT;
}

bool BitCountChunk::canInsertAt(const Ptr<const Chunk>& chunk, b offset) const
{
    return chunk->getChunkType() == CT_BITCOUNT;
}

void BitCountChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    const auto& bitCountChunk = staticPtrCast<const BitCountChunk>(chunk);
    length += bitCountChunk->length;
}

void BitCountChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    const auto& bitCountChunk = staticPtrCast<const BitCountChunk>(chunk);
    length += bitCountChunk->length;
}

void BitCountChunk::doInsertAt(const Ptr<const Chunk>& chunk, b offset)
{
    const auto& bitCountChunk = staticPtrCast<const BitCountChunk>(chunk);
    length += bitCountChunk->length;
}

void BitCountChunk::doRemoveAtFront(b length)
{
    this->length -= length;
}

void BitCountChunk::doRemoveAtBack(b length)
{
    this->length -= length;
}

void BitCountChunk::doRemoveAt(b offset, b length)
{
    this->length -= length;
}

std::ostream& BitCountChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(data, (int)data);
    return stream;
}

} // namespace

