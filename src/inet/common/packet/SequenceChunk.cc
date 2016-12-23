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

#include "inet/common/packet/SequenceChunk.h"

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

SequenceChunk::SequenceChunk(const std::deque<std::shared_ptr<Chunk>>& chunks) :
    Chunk(),
    chunks(chunks)
{
}

std::shared_ptr<Chunk> SequenceChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    assert(chunk->isImmutable());
    auto sequenceChunk = std::make_shared<SequenceChunk>();
    sequenceChunk->insertAtEnd(std::make_shared<SliceChunk>(chunk, offset, length));
    return sequenceChunk;
}

std::deque<std::shared_ptr<Chunk>> SequenceChunk::dupChunks() const
{
    std::deque<std::shared_ptr<Chunk>> copies;
    for (auto& chunk : chunks)
        copies.push_back(chunk->isImmutable() ? chunk : chunk->dupShared());
    return copies;
}

void SequenceChunk::setChunks(const std::deque<std::shared_ptr<Chunk>>& chunks)
{
    assertMutable();
    this->chunks = chunks;
}

void SequenceChunk::markImmutable()
{
    Chunk::markImmutable();
    for (auto& chunk : chunks)
        chunk->markImmutable();
}

int64_t SequenceChunk::getChunkLength() const
{
    int64_t length = 0;
    for (auto& chunk : chunks)
        length += chunk->getChunkLength();
    return length;
}

void SequenceChunk::seekIterator(Iterator& iterator, int64_t offset) const
{
    iterator.setPosition(offset);
    if (offset == 0)
        iterator.setIndex(0);
    else {
        int startIndex = getStartIndex(iterator);
        int endIndex = getEndIndex(iterator);
        int increment = getIndexIncrement(iterator);
        int64_t p = 0;
        for (int i = startIndex; i != endIndex + increment; i += increment) {
            p += chunks[i]->getChunkLength();
            if (p == offset) {
                iterator.setIndex(i + 1);
                return;
            }
        }
        iterator.setIndex(-1);
    }
}

void SequenceChunk::moveIterator(Iterator& iterator, int64_t length) const
{
    iterator.setPosition(iterator.getPosition() + length);
    if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size() && getElementChunk(iterator)->getChunkLength() == length)
        iterator.setIndex(iterator.getIndex() + 1);
    else
        iterator.setIndex(-1);
}

std::shared_ptr<Chunk> SequenceChunk::peekSequenceChunk1(const Iterator& iterator, int64_t length) const
{
    if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size()) {
        const auto& chunk = getElementChunk(iterator);
        if (length == -1 || chunk->getChunkLength() == length)
            return chunk;
    }
    return nullptr;
}

std::shared_ptr<Chunk> SequenceChunk::peekSequenceChunk2(const Iterator& iterator, int64_t length) const
{
    int position = 0;
    int startIndex = getStartIndex(iterator);
    int endIndex = getEndIndex(iterator);
    int increment = getIndexIncrement(iterator);
    for (int i = startIndex; i != endIndex + increment; i += increment) {
        auto& chunk = chunks[i];
        int64_t chunkLength = chunk->getChunkLength();
        if (iterator.getPosition() == position && (length == -1 || length == chunk->getChunkLength()))
            return chunk;
        else if (position <= iterator.getPosition() && iterator.getPosition() < position + chunkLength &&
                 (length == -1 || iterator.getPosition() + length <= position + chunkLength))
            return chunk->peek(ForwardIterator(iterator.getPosition() - position, -1), length);
        position += chunkLength;
    }
    return nullptr;
}

void SequenceChunk::doInsertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkLength() > 0);
    if (chunks.empty())
        chunks.push_front(chunk);
    else {
        auto& firstChunk = chunks.front();
        if (!firstChunk->canInsertAtBeginning(chunk))
            chunks.push_front(chunk);
        else {
            if (firstChunk.use_count() == 1)
                firstChunk->markMutableIfExclusivelyOwned();
            else
                firstChunk = firstChunk->dupShared();
            firstChunk->insertAtBeginning(chunk);
            firstChunk->markImmutable();
            chunks.front() = firstChunk->peek(0, firstChunk->getChunkLength());
        }
    }
}

void SequenceChunk::doInsertToBeginning(const std::shared_ptr<SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == TYPE_SEQUENCE) {
        auto sequenceChunk = std::static_pointer_cast<SequenceChunk>(sliceChunk->getChunk());
        int64_t offset = sequenceChunk->getChunkLength();
        int64_t sliceChunkBegin = sliceChunk->getOffset();
        int64_t sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (auto it = sequenceChunk->chunks.rbegin(); it != sequenceChunk->chunks.rend(); it++) {
            auto& elementChunk = *it;
            offset -= elementChunk->getChunkLength();
            int64_t chunkBegin = offset;
            int64_t chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertToBeginning(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertToBeginning(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertToBeginning(elementChunk->peek(0, sliceChunkEnd - chunkBegin));
        }
    }
    else
        doInsertToBeginning(std::static_pointer_cast<Chunk>(sliceChunk));
}

void SequenceChunk::doInsertToBeginning(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        doInsertToBeginning(*it);
}

void SequenceChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == TYPE_SLICE)
        doInsertToBeginning(std::static_pointer_cast<SliceChunk>(chunk));
    else if (chunk->getChunkType() == TYPE_SEQUENCE)
        doInsertToBeginning(std::static_pointer_cast<SequenceChunk>(chunk));
    else
        doInsertToBeginning(chunk);
}

void SequenceChunk::doInsertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkLength() > 0);
    if (chunks.empty())
        chunks.push_back(chunk);
    else {
        auto& lastChunk = chunks.back();
        if (!lastChunk->canInsertAtEnd(chunk))
            chunks.push_back(chunk);
        else {
            if (lastChunk.use_count() == 1)
                lastChunk->markMutableIfExclusivelyOwned();
            else
                lastChunk = lastChunk->dupShared();
            lastChunk->insertAtEnd(chunk);
            lastChunk->markImmutable();
            chunks.back() = lastChunk->peek(0, lastChunk->getChunkLength());
        }
    }
}

void SequenceChunk::doInsertToEnd(const std::shared_ptr<SliceChunk>& sliceChunk)
{
    if (sliceChunk->getChunk()->getChunkType() == TYPE_SEQUENCE) {
        auto sequenceChunk = std::static_pointer_cast<SequenceChunk>(sliceChunk->getChunk());
        int64_t offset = 0;
        int64_t sliceChunkBegin = sliceChunk->getOffset();
        int64_t sliceChunkEnd = sliceChunk->getOffset() + sliceChunk->getChunkLength();
        for (auto& elementChunk : sequenceChunk->chunks) {
            int64_t chunkBegin = offset;
            int64_t chunkEnd = offset + elementChunk->getChunkLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doInsertToEnd(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doInsertToEnd(elementChunk->peek(sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doInsertToEnd(elementChunk->peek(0, sliceChunkEnd - chunkBegin));
            offset += elementChunk->getChunkLength();
        }
    }
    else
        doInsertToEnd(std::static_pointer_cast<Chunk>(sliceChunk));
}

void SequenceChunk::doInsertToEnd(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto& chunk : chunk->chunks)
        doInsertToEnd(chunk);
}

void SequenceChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == TYPE_SLICE)
        doInsertToEnd(std::static_pointer_cast<SliceChunk>(chunk));
    else if (chunk->getChunkType() == TYPE_SEQUENCE)
        doInsertToEnd(std::static_pointer_cast<SequenceChunk>(chunk));
    else
        doInsertToEnd(chunk);
}

void SequenceChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= getChunkLength());
    handleChange();
    auto it = chunks.begin();
    while (it != chunks.end()) {
        auto chunk = *it;
        int64_t chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
        }
        else {
            *it = std::make_shared<SliceChunk>(chunk, length, chunkLength - length);
            length = 0;
        }
        if (length == 0)
            break;
    }
    chunks.erase(chunks.begin(), it);
}

void SequenceChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= getChunkLength());
    handleChange();
    auto it = chunks.rbegin();
    while (it != chunks.rend()) {
        auto chunk = *it;
        int64_t chunkLength = chunk->getChunkLength();
        if (chunkLength <= length) {
            it++;
            length -= chunkLength;
        }
        else {
            *it = std::make_shared<SliceChunk>(chunk, 0, chunkLength - length);
            length = 0;
        }
        if (length == 0)
            break;
    }
    chunks.erase((++it).base(), chunks.end());
}

std::shared_ptr<Chunk> SequenceChunk::peek(const Iterator& iterator, int64_t length) const
{
    int64_t chunkLength = getChunkLength();
    assert(0 <= iterator.getPosition() && iterator.getPosition() <= chunkLength);
    if (length == 0 || (iterator.getPosition() == chunkLength && length == -1))
        return nullptr;
    // NOTE: if length is -1 we return the child chunk instead of this
    else if (iterator.getPosition() == 0 && length == chunkLength)
        return const_cast<SequenceChunk *>(this)->shared_from_this();
    else {
        if (auto chunk = peekSequenceChunk1(iterator, length))
            return chunk;
        if (auto chunk = peekSequenceChunk2(iterator, length))
            return chunk;
        return doPeek<SliceChunk>(iterator, length);
    }
}

std::string SequenceChunk::str() const
{
    std::ostringstream os;
    os << "[";
    bool first = true;
    for (auto& chunk : chunks) {
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
