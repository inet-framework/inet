//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/SequenceChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"

namespace inet {

Register_Class(SequenceChunk);

SequenceChunk::SequenceChunk() :
    Chunk()
{
}

SequenceChunk::SequenceChunk(const SequenceChunk& other) :
    Chunk(other),
    chunks(other.isImmutable() ? other.chunks : other.dupChunks())
{
}

SequenceChunk::SequenceChunk(const std::deque<Ptr<const Chunk>>& chunks) :
    Chunk(),
    chunks(chunks)
{
}

void SequenceChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(chunks.size());
    for (const auto& chunk : chunks)
        buffer->packObject(const_cast<Chunk *>(chunk.get()));
}

void SequenceChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    size_t size;
    buffer->unpack(size);
    chunks.clear();
    for (size_t i = 0; i < size; i++) {
        const auto& chunk = check_and_cast<Chunk *>(buffer->unpackObject())->shared_from_this();
        chunks.push_back(chunk);
    }
}

void SequenceChunk::forEachChild(cVisitor *v)
{
    Chunk::forEachChild(v);
    for (const auto& chunk : chunks)
        v->visit(const_cast<Chunk *>(chunk.get()));
}

bool SequenceChunk::containsSameData(const Chunk& other) const
{
    if (&other == this)
        return true;
    else if (!Chunk::containsSameData(other))
        return false;
    else {
        auto otherSequence = static_cast<const SequenceChunk *>(&other);
        if (chunks.size() != otherSequence->chunks.size())
            return false;
        else {
            for (auto i = 0; i < (int)chunks.size(); i++)
                if (!chunks[i]->containsSameData(*otherSequence->chunks[i].get()))
                    return false;
            return true;
        }
    }
}

const Ptr<Chunk> SequenceChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    b chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(b(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == b(0) || (iterator.getPosition() == chunkLength && length < b(0))) {
        if (predicate == nullptr || predicate(nullptr))
            return EmptyChunk::getEmptyChunk(flags);
    }
    // 2. peeking the whole part returns this chunk only if length is also specified
    if (iterator.getPosition() == b(0) && length == chunkLength) {
        auto result = const_cast<SequenceChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking a part represented by an element chunk with its index returns that element chunk
    if (iterator.getIndex() != -1 && iterator.getIndex() != (int)chunks.size()) {
        // KLUDGE constPtrCast<Chunk>
        const auto& chunk = constPtrCast<Chunk>(getElementChunk(iterator));
        auto chunkLength = chunk->getChunkLength();
        if (-length >= chunkLength || length == chunkLength) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
    }
    // 4. peeking a part represented by an element chunk returns part of that element chunk
    b position = b(0);
    for (size_t i = 0; i < chunks.size(); i++) {
        // KLUDGE constPtrCast<Chunk>
        const auto& chunk = constPtrCast<Chunk>(chunks[getElementIndex(iterator.isForward(), i)]);
        b elementChunkLength = chunk->getChunkLength();
        // 4.1 peeking the whole part of an element chunk returns that element chunk
        if (iterator.getPosition() == position && (-length >= elementChunkLength || length == elementChunkLength)) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
        // 4.2 peeking a part of an element chunk returns the part of that element chunk
        if (position <= iterator.getPosition() && iterator.getPosition() < position + elementChunkLength &&
            (-length >= elementChunkLength || iterator.getPosition() + length <= position + elementChunkLength))
            return chunk->peekUnchecked(predicate, converter, Iterator(iterator.isForward(), iterator.getPosition() - position, -1), length, flags);
        position += elementChunkLength;
    }
    // 5. peeking without conversion returns a SequenceChunk
    if (converter == nullptr)
        return peekConverted<SequenceChunk>(iterator, length, flags);
    // 6. peeking with conversion
    return converter(const_cast<SequenceChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> SequenceChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    auto chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    auto sequenceChunk = makeShared<SequenceChunk>();
    auto sliceChunk = makeShared<SliceChunk>(chunk, offset, length < b(0) ? std::min(-length, chunkLength - offset) : length);
    sliceChunk->markImmutable();
    sequenceChunk->insertAtBack(sliceChunk);
    return sequenceChunk;
}

std::deque<Ptr<const Chunk>> SequenceChunk::dupChunks() const
{
    std::deque<Ptr<const Chunk>> copies;
    for (const auto& chunk : chunks)
        copies.push_back(chunk->isImmutable() ? chunk : staticPtrCast<const Chunk>(chunk->dupShared()));
    return copies;
}

void SequenceChunk::setChunks(const std::deque<Ptr<const Chunk>>& chunks)
{
    handleChange();
    this->chunks = chunks;
}

bool SequenceChunk::isIncomplete() const
{
    if (Chunk::isIncomplete())
        return true;
    for (const auto& chunk : chunks)
        if (chunk->isIncomplete())
            return true;
    return false;
}

bool SequenceChunk::isIncorrect() const
{
    if (Chunk::isIncorrect())
        return true;
    for (const auto& chunk : chunks)
        if (chunk->isIncorrect())
            return true;
    return false;
}

bool SequenceChunk::isImproperlyRepresented() const
{
    if (Chunk::isImproperlyRepresented())
        return true;
    for (const auto& chunk : chunks)
        if (chunk->isImproperlyRepresented())
            return true;
    return false;
}

b SequenceChunk::getChunkLength() const
{
    b length = b(0);
    for (const auto& chunk : chunks) {
        auto chunkLength = chunk->getChunkLength();
        CHUNK_CHECK_IMPLEMENTATION(chunkLength > b(0));
        length += chunkLength;
    }
    return length;
}

void SequenceChunk::seekIterator(Iterator& iterator, b offset) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getChunkLength(), "offset is out of range");
    iterator.setPosition(offset);
    if (offset == b(0))
        iterator.setIndex(0);
    else {
        b p = b(0);
        for (size_t i = 0; i < chunks.size(); i++) {
            const auto& chunk = chunks[getElementIndex(iterator.isForward(), i)];
            p += chunk->getChunkLength();
            if (p == offset) {
                iterator.setIndex(i + 1);
                return;
            }
            else if (p > offset) {
                iterator.setIndex(-1);
                return;
            }
        }
        CHUNK_CHECK_IMPLEMENTATION(false);
    }
}

void SequenceChunk::moveIterator(Iterator& iterator, b length) const
{
    iterator.setPosition(iterator.getPosition() + length);
    if (iterator.getIndex() != -1 && iterator.getIndex() != (int)chunks.size() && getElementChunk(iterator)->getChunkLength() == length)
        iterator.setIndex(iterator.getIndex() + 1);
    else
        iterator.setIndex(-1);
}

void SequenceChunk::doInsertChunkAtFront(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
    if (chunks.empty())
        chunks.push_front(chunk);
    else {
        const auto& firstChunk = chunks.front();
        if (!firstChunk->canInsertAtFront(chunk))
            chunks.push_front(chunk);
        else {
            const auto& mutableFirstChunk = makeExclusivelyOwnedMutableChunk(firstChunk);
            mutableFirstChunk->insertAtFront(chunk);
            mutableFirstChunk->markImmutable();
            chunks.front() = mutableFirstChunk->simplify();
        }
    }
}

void SequenceChunk::doInsertSliceChunkAtFront(const Ptr<const SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == CT_SEQUENCE) {
        auto sequenceChunk = staticPtrCast<SequenceChunk>(sliceChunk->getChunk());
        b offset = sequenceChunk->getChunkLength();
        b sliceChunkBegin = sliceChunk->getOffset();
        b sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (auto it = sequenceChunk->chunks.rbegin(); it != sequenceChunk->chunks.rend(); it++) {
            const auto& elementChunk = *it;
            offset -= elementChunk->getChunkLength();
            b chunkBegin = offset;
            b chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertChunkAtFront(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkEnd < chunkEnd)
                doInsertChunkAtFront(staticPtrCast<const Chunk>(elementChunk->peek(sliceChunkBegin - chunkBegin, sliceChunkEnd - sliceChunkBegin)));
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertChunkAtFront(staticPtrCast<const Chunk>(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin)));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertChunkAtFront(staticPtrCast<const Chunk>(elementChunk->peek(b(0), sliceChunkEnd - chunkBegin)));
            // otherwise the element chunk is out of the slice, therefore it's ignored
        }
    }
    else
        doInsertChunkAtFront(staticPtrCast<const Chunk>(sliceChunk));
}

void SequenceChunk::doInsertSequenceChunkAtFront(const Ptr<const SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        doInsertChunkAtFront(*it);
}

void SequenceChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    if (chunk->getChunkType() == CT_SLICE)
        doInsertSliceChunkAtFront(staticPtrCast<const SliceChunk>(chunk));
    else if (chunk->getChunkType() == CT_SEQUENCE)
        doInsertSequenceChunkAtFront(staticPtrCast<const SequenceChunk>(chunk));
    else
        doInsertChunkAtFront(chunk);
}

void SequenceChunk::doInsertChunkAtBack(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
    if (chunks.empty())
        chunks.push_back(chunk);
    else {
        const auto& lastChunk = chunks.back();
        if (!lastChunk->canInsertAtBack(chunk))
            chunks.push_back(chunk);
        else {
            const auto& mutableLastChunk = makeExclusivelyOwnedMutableChunk(lastChunk);
            mutableLastChunk->insertAtBack(chunk);
            mutableLastChunk->markImmutable();
            chunks.back() = mutableLastChunk->simplify();
        }
    }
}

void SequenceChunk::doInsertSliceChunkAtBack(const Ptr<const SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == CT_SEQUENCE) {
        auto sequenceChunk = staticPtrCast<SequenceChunk>(sliceChunk->getChunk());
        b offset = b(0);
        b sliceChunkBegin = sliceChunk->getOffset();
        b sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (const auto& elementChunk : sequenceChunk->chunks) {
            b chunkBegin = offset;
            b chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertChunkAtBack(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkEnd < chunkEnd)
                doInsertChunkAtBack(staticPtrCast<const Chunk>(elementChunk->peek(sliceChunkBegin - chunkBegin, sliceChunkEnd - sliceChunkBegin)));
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertChunkAtBack(staticPtrCast<const Chunk>(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin)));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertChunkAtBack(staticPtrCast<const Chunk>(elementChunk->peek(b(0), sliceChunkEnd - chunkBegin)));
            // otherwise the element chunk is out of the slice, therefore it's ignored
            offset += elementChunk->getChunkLength();
        }
    }
    else
        doInsertChunkAtBack(staticPtrCast<const Chunk>(sliceChunk));
}

void SequenceChunk::doInsertSequenceChunkAtBack(const Ptr<const SequenceChunk>& chunk)
{
    for (const auto& elementChunk : chunk->chunks)
        doInsertChunkAtBack(elementChunk);
}

void SequenceChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    if (chunk->getChunkType() == CT_SLICE)
        doInsertSliceChunkAtBack(staticPtrCast<const SliceChunk>(chunk));
    else if (chunk->getChunkType() == CT_SEQUENCE)
        doInsertSequenceChunkAtBack(staticPtrCast<const SequenceChunk>(chunk));
    else
        doInsertChunkAtBack(chunk);
}

void SequenceChunk::doRemoveAtFront(b length)
{
    auto it = chunks.begin();
    while (it != chunks.end()) {
        auto chunk = *it;
        b chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
            if (length == b(0))
                break;
        }
        else {
            *it = chunk->peek(length, chunkLength - length);
            break;
        }
    }
    chunks.erase(chunks.begin(), it);
}

void SequenceChunk::doRemoveAtBack(b length)
{
    auto it = chunks.rbegin();
    while (it != chunks.rend()) {
        auto chunk = *it;
        b chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
            if (length == b(0))
                break;
        }
        else {
            *it = chunk->peek(b(0), chunkLength - length);
            break;
        }
    }
    chunks.erase(it.base(), chunks.end());
}

std::ostream& SequenceChunk::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "[";
    bool first = true;
    for (const auto& chunk : chunks) {
        if (!first)
            stream << " | ";
        else
            first = false;
        chunk->printToStream(stream, level + 1, evFlags);
    }
    return stream << "]";
}

} // namespace

