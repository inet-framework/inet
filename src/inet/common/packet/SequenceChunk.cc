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

SequenceChunk::SequenceIterator::SequenceIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index, int64_t position) :
    Iterator(position),
    chunk(chunk),
    index(index)
{
}

SequenceChunk::SequenceIterator::SequenceIterator(const SequenceIterator& other) :
    Iterator(other),
    chunk(other.chunk),
    index(other.index)
{
}

void SequenceChunk::SequenceIterator::seek(int64_t byteOffset)
{
    position = byteOffset;
    if (byteOffset == 0)
        index = 0;
    else {
        int startIndex = getStartIndex();
        int endIndex = getEndIndex();
        int increment = getIndexIncrement();
        int64_t p = 0;
        for (int i = startIndex; i != endIndex + increment; i += increment) {
            p += chunk->chunks[i]->getByteLength();
            if (p == byteOffset) {
                index = i + 1;
                return;
            }
        }
        index = -1;
    }
}

void SequenceChunk::SequenceIterator::move(int64_t byteLength)
{
    position += byteLength;
    if (index != -1 && index != chunk->chunks.size() && getElementChunk()->getByteLength() == byteLength)
        index++;
    else
        index = -1;
}

SequenceChunk::ForwardIterator::ForwardIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index, int64_t position) :
    SequenceIterator(chunk, index, position)
{
}

SequenceChunk::ForwardIterator::ForwardIterator(const ForwardIterator& other) :
    SequenceIterator(other)
{
}

SequenceChunk::BackwardIterator::BackwardIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index, int64_t position) :
    SequenceIterator(chunk, index, position)
{
}

SequenceChunk::BackwardIterator::BackwardIterator(const ForwardIterator& other) :
    SequenceIterator(other)
{
}

SequenceChunk::SequenceChunk(const SequenceChunk& other) :
    Chunk(other),
    chunks(other.isImmutable() ? other.chunks : other.cloneChunks())
{
}

void SequenceChunk::makeImmutable()
{
    Chunk::makeImmutable();
    for (auto& chunk : chunks)
        chunk->makeImmutable();
}

std::vector<std::shared_ptr<Chunk> > SequenceChunk::cloneChunks() const
{
    std::vector<std::shared_ptr<Chunk> > clones;
    for (auto& chunk : chunks)
        clones.push_back(chunk->isImmutable() ? chunk : chunk->dupShared());
    return clones;
}

int64_t SequenceChunk::getByteLength() const
{
    int64_t byteLength = 0;
    for (auto& chunk : chunks)
        byteLength += chunk->getByteLength();
    return byteLength;
}

std::shared_ptr<Chunk> SequenceChunk::peekWithIterator(const SequenceIterator& iterator, int64_t byteLength) const
{
    if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size()) {
        const auto& chunk = iterator.getElementChunk();
        if (byteLength == -1 || chunk->getByteLength() == byteLength)
            return chunk;
    }
    return nullptr;
}

std::shared_ptr<Chunk> SequenceChunk::peekWithLinearSearch(const SequenceIterator& iterator, int64_t byteLength) const
{
    int position = 0;
    int startIndex = iterator.getStartIndex();
    int endIndex = iterator.getEndIndex();
    int increment = iterator.getIndexIncrement();
    for (int i = startIndex; i != endIndex + increment; i += increment) {
        auto& chunk = chunks[i];
        if (iterator.getPosition() == position && (byteLength == -1 || byteLength == chunk->getByteLength()))
            return chunk;
        position += chunk->getByteLength();
    }
    return nullptr;
}

std::shared_ptr<Chunk> SequenceChunk::peek(const Iterator& iterator, int64_t byteLength) const
{
    if (iterator.getPosition() == 0 && byteLength == getByteLength())
        return const_cast<SequenceChunk *>(this)->shared_from_this();
    else
        return peek<SliceChunk>(static_cast<const SequenceIterator&>(iterator), byteLength);
}

bool SequenceChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getByteLength() <= 0)
        throw cRuntimeError("Invalid chunk length: %li", chunk->getByteLength());
    chunks.insert(chunks.begin(), chunk);
    return true;
}

void SequenceChunk::insertToBeginning(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        insertToBeginning(*it);
}

void SequenceChunk::prepend(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    if (!flatten)
        insertToBeginning(chunk);
    else if (auto sliceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        insertToBeginning(sliceChunk);
    else if (auto sequenceChunk = std::dynamic_pointer_cast<SequenceChunk>(chunk))
        insertToBeginning(sequenceChunk);
    else
        insertToBeginning(chunk);
}

bool SequenceChunk::insertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getByteLength() <= 0)
        throw cRuntimeError("Invalid chunk length: %li", chunk->getByteLength());
    if (chunks.size() == 0)
        chunks.push_back(chunk);
    else {
        auto& lastChunk = chunks.back();
        if (lastChunk->insertToEnd(chunk)) {
            if (auto sliceChunk = std::dynamic_pointer_cast<SliceChunk>(lastChunk)) {
                if (sliceChunk->getByteOffset() == 0 && sliceChunk->getByteLength() == sliceChunk->getChunk()->getByteLength())
                    chunks.back() = sliceChunk->getChunk();
            }
        }
        else
            chunks.push_back(chunk);
    }
    return true;
}

void SequenceChunk::insertToEnd(const std::shared_ptr<SliceChunk>& sliceChunk)
{
    if (auto sequenceChunk = std::dynamic_pointer_cast<SequenceChunk>(sliceChunk->getChunk())) {
        int64_t byteOffset = 0;
        for (auto& elementChunk : sequenceChunk->chunks) {
            int64_t sliceChunkBegin = sliceChunk->getByteOffset();
            int64_t sliceChunkEnd = sliceChunk->getByteOffset() + sliceChunk->getByteLength();
            int64_t chunkBegin = byteOffset;
            int64_t chunkEnd = byteOffset + elementChunk->getByteLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                insertToEnd(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                insertToEnd(std::make_shared<SliceChunk>(elementChunk, sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                insertToEnd(std::make_shared<SliceChunk>(elementChunk, 0, chunkEnd - sliceChunkEnd));
            byteOffset += elementChunk->getByteLength();
        }
    }
    else
        insertToEnd(std::static_pointer_cast<Chunk>(sliceChunk));
}

void SequenceChunk::insertToEnd(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto& chunk : chunk->chunks)
        insertToEnd(chunk);
}

void SequenceChunk::append(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    if (!flatten)
        insertToEnd(chunk);
    else if (auto sliceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        insertToEnd(sliceChunk);
    else if (auto sequenceChunk = std::dynamic_pointer_cast<SequenceChunk>(chunk))
        insertToEnd(sequenceChunk);
    else
        insertToEnd(chunk);
}

bool SequenceChunk::removeFromBeginning(int64_t byteLength)
{
    handleChange();
    auto it = chunks.begin();
    while (it != chunks.end()) {
        auto chunk = *it;
        int64_t chunkByteLength = chunk->getByteLength();
        if (chunkByteLength <= byteLength) {
            it++;
            byteLength -= chunkByteLength;
        }
        else {
            *it = std::make_shared<SliceChunk>(chunk, byteLength, chunkByteLength - byteLength);
            byteLength = 0;
        }
        if (byteLength == 0)
            break;
    }
    chunks.erase(chunks.begin(), it);
    return true;
}

bool SequenceChunk::removeFromEnd(int64_t byteLength)
{
    handleChange();
    auto it = chunks.rbegin();
    while (it != chunks.rend()) {
        auto chunk = *it;
        int64_t chunkByteLength = chunk->getByteLength();
        if (chunkByteLength <= byteLength) {
            it++;
            byteLength -= chunkByteLength;
        }
        else {
            *it = std::make_shared<SliceChunk>(chunk, 0, chunkByteLength - byteLength);
            byteLength = 0;
        }
        if (byteLength == 0)
            break;
    }
    chunks.erase((++it).base(), chunks.end());
    return true;
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
