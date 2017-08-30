//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

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

void SequenceChunk::forEachChild(cVisitor *v)
{
    for (const auto& chunk : chunks)
        v->visit(const_cast<Chunk *>(chunk.get()));
}

const Ptr<Chunk> SequenceChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
{
    bit chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(bit(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1))) {
        if (predicate == nullptr || predicate(nullptr))
            return nullptr;
    }
    // 2. peeking the whole part returns this chunk only if length is also specified
    if (iterator.getPosition() == bit(0) && length == chunkLength) {
        auto result = const_cast<SequenceChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking a part represented by an element chunk with its index returns that element chunk
    if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size()) {
        // KLUDGE: TODO: std::const_pointer_cast<Chunk>
        const auto& chunk = std::const_pointer_cast<Chunk>(getElementChunk(iterator));
        if (length == bit(-1) || chunk->getChunkLength() == length) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
    }
    // 4. peeking a part represented by an element chunk returns that element chunk
    bit position = bit(0);
    for (size_t i = 0; i < chunks.size(); i++) {
        // KLUDGE: TODO: std::const_pointer_cast<Chunk>
        const auto& chunk = std::const_pointer_cast<Chunk>(chunks[getElementIndex(iterator.isForward(), i)]);
        bit chunkLength = chunk->getChunkLength();
        // 4.1 peeking the whole part of an element chunk returns that element chunk
        if (iterator.getPosition() == position && (length == bit(-1) || length == chunk->getChunkLength())) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
        // 4.2 peeking a part of an element chunk returns the part of that element chunk
        if (position <= iterator.getPosition() && iterator.getPosition() < position + chunkLength &&
            (length == bit(-1) || iterator.getPosition() + length <= position + chunkLength))
            return chunk->peekUnchecked(predicate, converter, ForwardIterator(iterator.getPosition() - position, -1), length, flags);
        position += chunkLength;
    }
    // 5. peeking without conversion returns a SequenceChunk
    if (converter == nullptr)
        return peekConverted<SequenceChunk>(iterator, length, flags);
    // 6. peeking with conversion
    return converter(const_cast<SequenceChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> SequenceChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, bit offset, bit length, int flags)
{
    auto sequenceChunk = std::make_shared<SequenceChunk>();
    sequenceChunk->insertAtEnd(std::make_shared<SliceChunk>(chunk, offset, length));
    return sequenceChunk;
}

std::deque<Ptr<const Chunk>> SequenceChunk::dupChunks() const
{
    std::deque<Ptr<const Chunk>> copies;
    for (const auto& chunk : chunks)
        copies.push_back(chunk->isImmutable() ? chunk : chunk->dupShared());
    return copies;
}

void SequenceChunk::setChunks(const std::deque<Ptr<const Chunk>>& chunks)
{
    handleChange();
    this->chunks = chunks;
}

bool SequenceChunk::isMutable() const
{
    if (Chunk::isMutable())
        return true;
    for (const auto& chunk : chunks)
        if (chunk->isMutable())
            return true;
    return false;
}

void SequenceChunk::markImmutable()
{
    Chunk::markImmutable();
    for (const auto& chunk : chunks)
        std::const_pointer_cast<Chunk>(chunk)->markImmutable();
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

bit SequenceChunk::getChunkLength() const
{
    bit length = bit(0);
    for (const auto& chunk : chunks) {
        auto chunkLength = chunk->getChunkLength();
        CHUNK_CHECK_IMPLEMENTATION(chunkLength > bit(0));
        length += chunkLength;
    }
    return length;
}

void SequenceChunk::seekIterator(Iterator& iterator, bit offset) const
{
    CHUNK_CHECK_USAGE(bit(0) <= offset && offset <= getChunkLength(), "offset is out of range");
    iterator.setPosition(offset);
    if (offset == bit(0))
        iterator.setIndex(0);
    else {
        bit p = bit(0);
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

void SequenceChunk::moveIterator(Iterator& iterator, bit length) const
{
    iterator.setPosition(iterator.getPosition() + length);
    if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size() && getElementChunk(iterator)->getChunkLength() == length)
        iterator.setIndex(iterator.getIndex() + 1);
    else
        iterator.setIndex(-1);
}

void SequenceChunk::doInsertToBeginning(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > bit(0), "chunk is empty");
    if (chunks.empty())
        chunks.push_front(chunk);
    else {
        const auto& firstChunk = chunks.front();
        if (!firstChunk->canInsertAtBeginning(chunk))
            chunks.push_front(chunk);
        else {
            const auto& mutableFirstChunk = makeExclusivelyOwnedMutableChunk(firstChunk);
            mutableFirstChunk->insertAtBeginning(chunk);
            mutableFirstChunk->markImmutable();
            chunks.front() = mutableFirstChunk->simplify();
        }
    }
}

void SequenceChunk::doInsertToBeginning(const Ptr<const SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == CT_SEQUENCE) {
        auto sequenceChunk = std::static_pointer_cast<SequenceChunk>(sliceChunk->getChunk());
        bit offset = sequenceChunk->getChunkLength();
        bit sliceChunkBegin = sliceChunk->getOffset();
        bit sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (auto it = sequenceChunk->chunks.rbegin(); it != sequenceChunk->chunks.rend(); it++) {
            const auto& elementChunk = *it;
            offset -= elementChunk->getChunkLength();
            bit chunkBegin = offset;
            bit chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertToBeginning(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkEnd < chunkEnd)
                doInsertToBeginning(elementChunk->peek(sliceChunkBegin - chunkBegin, sliceChunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertToBeginning(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertToBeginning(elementChunk->peek(bit(0), sliceChunkEnd - chunkBegin));
            else
                CHUNK_CHECK_IMPLEMENTATION(false);
        }
    }
    else
        doInsertToBeginning(std::static_pointer_cast<const Chunk>(sliceChunk));
}

void SequenceChunk::doInsertToBeginning(const Ptr<const SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        doInsertToBeginning(*it);
}

void SequenceChunk::insertAtBeginning(const Ptr<const Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == CT_SLICE)
        doInsertToBeginning(std::static_pointer_cast<const SliceChunk>(chunk));
    else if (chunk->getChunkType() == CT_SEQUENCE)
        doInsertToBeginning(std::static_pointer_cast<const SequenceChunk>(chunk));
    else
        doInsertToBeginning(chunk);
}

void SequenceChunk::doInsertToEnd(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > bit(0), "chunk is empty");
    if (chunks.empty())
        chunks.push_back(chunk);
    else {
        const auto& lastChunk = chunks.back();
        if (!lastChunk->canInsertAtEnd(chunk))
            chunks.push_back(chunk);
        else {
            const auto& mutableLastChunk = makeExclusivelyOwnedMutableChunk(lastChunk);
            mutableLastChunk->insertAtEnd(chunk);
            mutableLastChunk->markImmutable();
            chunks.back() = mutableLastChunk->simplify();
        }
    }
}

void SequenceChunk::doInsertToEnd(const Ptr<const SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == CT_SEQUENCE) {
        auto sequenceChunk = std::static_pointer_cast<SequenceChunk>(sliceChunk->getChunk());
        bit offset = bit(0);
        bit sliceChunkBegin = sliceChunk->getOffset();
        bit sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (const auto& elementChunk : sequenceChunk->chunks) {
            bit chunkBegin = offset;
            bit chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertToEnd(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkEnd < chunkEnd)
                doInsertToEnd(elementChunk->peek(sliceChunkBegin - chunkBegin, sliceChunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertToEnd(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertToEnd(elementChunk->peek(bit(0), sliceChunkEnd - chunkBegin));
            else {
                // chunk is out of slice, ignored
            }
            offset += elementChunk->getChunkLength();
        }
    }
    else
        doInsertToEnd(std::static_pointer_cast<const Chunk>(sliceChunk));
}

void SequenceChunk::doInsertToEnd(const Ptr<const SequenceChunk>& chunk)
{
    for (const auto& elementChunk : chunk->chunks)
        doInsertToEnd(elementChunk);
}

void SequenceChunk::insertAtEnd(const Ptr<const Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == CT_SLICE)
        doInsertToEnd(std::static_pointer_cast<const SliceChunk>(chunk));
    else if (chunk->getChunkType() == CT_SEQUENCE)
        doInsertToEnd(std::static_pointer_cast<const SequenceChunk>(chunk));
    else
        doInsertToEnd(chunk);
}

void SequenceChunk::removeFromBeginning(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    auto it = chunks.begin();
    while (it != chunks.end()) {
        auto chunk = *it;
        bit chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
            if (length == bit(0))
                break;
        }
        else {
            *it = chunk->peek(length, chunkLength - length);
            break;
        }
    }
    chunks.erase(chunks.begin(), it);
}

void SequenceChunk::removeFromEnd(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    auto it = chunks.rbegin();
    while (it != chunks.rend()) {
        auto chunk = *it;
        bit chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
            if (length == bit(0))
                break;
        }
        else {
            *it = chunk->peek(bit(0), chunkLength - length);
            break;
        }
    }
    chunks.erase(it.base(), chunks.end());
}

std::string SequenceChunk::str() const
{
    std::ostringstream os;
    os << "[";
    bool first = true;
    for (const auto& chunk : chunks) {
        if (!first)
            os << " | ";
        else
            first = false;
        os << chunk->str();
    }
    os << "]";
    return os.str();
}

} // namespace
