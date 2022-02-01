//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/BitsChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"

namespace inet {

Register_Class(BitsChunk);

BitsChunk::BitsChunk() :
    Chunk()
{
}

BitsChunk::BitsChunk(const std::vector<bool>& bits) :
    Chunk(),
    bits(bits)
{
}

void BitsChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(bits.size());
    for (auto bit : bits)
        buffer->pack(bit);
}

void BitsChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    size_t size;
    buffer->unpack(size);
    bits.clear();
    for (size_t i = 0; i < size; i++) {
        bool bit;
        buffer->unpack(bit);
        bits.push_back(bit);
    }
}

const Ptr<Chunk> BitsChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
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
        auto result = const_cast<BitsChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a BitsChunk
    if (converter == nullptr) {
        b startOffset = iterator.getPosition();
        b endOffset = iterator.getPosition() + (length < b(0) ? std::min(-length, chunkLength - iterator.getPosition()) : length);
        auto result = makeShared<BitsChunk>(std::vector<bool>(bits.begin() + b(startOffset).get(), bits.begin() + b(endOffset).get()));
        result->regionTags.copyTags(regionTags, iterator.getPosition(), b(0), result->getChunkLength());
        result->markImmutable();
        return result;
    }
    // 4. peeking with conversion
    return converter(const_cast<BitsChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> BitsChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    b resultLength = length < b(0) ? std::min(-length, chunkLength - offset) : length;
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= resultLength && resultLength <= chunkLength);
    MemoryOutputStream outputStream(chunkLength);
    Chunk::serialize(outputStream, chunk, offset, resultLength);
    // TODO optimize
    std::vector<bool> bits;
    outputStream.copyData(bits);
    return makeShared<BitsChunk>(bits);
}

void BitsChunk::setBits(const std::vector<bool>& bits)
{
    handleChange();
    this->bits = bits;
}

void BitsChunk::setBit(int index, bool bit)
{
    handleChange();
    bits.at(index) = bit;
}

bool BitsChunk::containsSameData(const Chunk& other) const
{
    return &other == this || (Chunk::containsSameData(other) && bits == static_cast<const BitsChunk *>(&other)->bits);
}

bool BitsChunk::canInsertAtFront(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BITS;
}

bool BitsChunk::canInsertAtBack(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BITS;
}

bool BitsChunk::canInsertAt(const Ptr<const Chunk>& chunk, b offset) const
{
    return chunk->getChunkType() == CT_BITS;
}

void BitsChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    const auto& bitsChunk = staticPtrCast<const BitsChunk>(chunk);
    bits.insert(bits.begin(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    const auto& bitsChunk = staticPtrCast<const BitsChunk>(chunk);
    bits.insert(bits.end(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::doInsertAt(const Ptr<const Chunk>& chunk, b offset)
{
    const auto& bitsChunk = staticPtrCast<const BitsChunk>(chunk);
    bits.insert(bits.begin() + b(offset).get(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::doRemoveAtFront(b length)
{
    bits.erase(bits.begin(), bits.begin() + b(length).get());
}

void BitsChunk::doRemoveAtBack(b length)
{
    bits.erase(bits.end() - b(length).get(), bits.end());
}

void BitsChunk::doRemoveAt(b offset, b length)
{
    bits.erase(bits.begin() + b(offset).get(), bits.begin() + b(offset).get() + b(length).get());
}

std::ostream& BitsChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE) {
        stream << ", " << EV_BOLD << "bits" << EV_NORMAL << " = {";
        for (auto bit : bits)
            stream << (bit ? "1" : "0");
        stream << "}";
    }
    return stream;
}

} // namespace

