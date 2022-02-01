//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/ByteCountChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

Register_Class(ByteCountChunk);

ByteCountChunk::ByteCountChunk() :
    Chunk(),
    length(-1),
    data('?')
{
}

ByteCountChunk::ByteCountChunk(B length, uint8_t data) :
    Chunk(),
    length(length),
    data(data)
{
    CHUNK_CHECK_USAGE(length >= B(0), "length is invalid");
}

void ByteCountChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(B(length).get());
    buffer->pack(data);
}

void ByteCountChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    int64_t l;
    buffer->unpack(l);
    length = B(l);
    buffer->unpack(data);
}

const Ptr<Chunk> ByteCountChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
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
        auto result = const_cast<ByteCountChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a ByteCountChunk or SliceChunk
    if (converter == nullptr) {
        // 3.a) peeking complete bytes without conversion returns a ByteCountChunk
        if (b(iterator.getPosition()).get() % 8 == 0 && (length < b(0) || length.get() % 8 == 0)) {
            auto chunk = makeShared<ByteCountChunk>(length < b(0) ? std::min(-length, chunkLength - iterator.getPosition()) : length);
            chunk->regionTags.copyTags(regionTags, iterator.getPosition(), b(0), chunk->getChunkLength());
            chunk->markImmutable();
            return chunk;
        }
        else
            // 3.b) peeking incomplete bytes without conversion returns a SliceChunk
            return peekConverted<SliceChunk>(iterator, length, flags);
    }
    // 4. peeking with conversion
    return converter(const_cast<ByteCountChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> ByteCountChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    b resultLength = length < b(0) ? std::min(-length, chunkLength - offset) : length;
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= resultLength && resultLength <= chunkLength);
    return makeShared<ByteCountChunk>(B(resultLength));
}

void ByteCountChunk::setLength(B length)
{
    CHUNK_CHECK_USAGE(length >= B(0), "length is invalid");
    handleChange();
    this->length = length;
}

void ByteCountChunk::setData(uint8_t data)
{
    handleChange();
    this->data = data;
}

bool ByteCountChunk::containsSameData(const Chunk& other) const
{
    return &other == this || (Chunk::containsSameData(other) && data == static_cast<const ByteCountChunk *>(&other)->data);
}

bool ByteCountChunk::canInsertAtFront(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTECOUNT;
}

bool ByteCountChunk::canInsertAtBack(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTECOUNT;
}

bool ByteCountChunk::canInsertAt(const Ptr<const Chunk>& chunk, b offset) const
{
    return chunk->getChunkType() == CT_BYTECOUNT;
}

void ByteCountChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    const auto& byteCountChunk = staticPtrCast<const ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    const auto& byteCountChunk = staticPtrCast<const ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::doInsertAt(const Ptr<const Chunk>& chunk, b offset)
{
    const auto& byteCountChunk = staticPtrCast<const ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::doRemoveAtFront(b length)
{
    this->length -= B(length);
}

void ByteCountChunk::doRemoveAtBack(b length)
{
    this->length -= B(length);
}

void ByteCountChunk::doRemoveAt(b offset, b length)
{
    this->length -= B(length);
}

std::ostream& ByteCountChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    Chunk::printFieldsToStream(stream, level, evFlags);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(data, (int)data);
    return stream;
}

} // namespace

