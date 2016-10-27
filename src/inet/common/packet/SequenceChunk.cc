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

SequenceChunk::SequenceChunk(const SequenceChunk& other) :
    Chunk(other),
    chunks(other.chunks)
{
}

SequenceChunk::Iterator::Iterator(const Iterator& other) :
    chunk(other.chunk),
    position(other.position),
    index(other.index)
{
}

void SequenceChunk::makeImmutable()
{
    Chunk::makeImmutable();
    for (auto& chunk : chunks)
        chunk->makeImmutable();
}

int64_t SequenceChunk::getByteLength() const
{
    int64_t byteLength = 0;
    for (auto& chunk : chunks)
        byteLength += chunk->getByteLength();
    return byteLength;
}

void SequenceChunk::Iterator::seek(int64_t byteOffset)
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
    }
}

void SequenceChunk::Iterator::move(int64_t byteLength)
{
    position += byteLength;
    if (index != -1 && index != chunk->chunks.size() && getChunk()->getByteLength() == byteLength)
        index++;
    else
        index = -1;
}

void SequenceChunk::prependChunk(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    chunks.insert(chunks.begin(), chunk);
}

void SequenceChunk::prependChunk(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        prependChunk(*it);
}

void SequenceChunk::prepend(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    if (!flatten)
        prependChunk(chunk);
    else if (auto sliceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        prependChunk(sliceChunk);
    else if (auto sequenceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        prependChunk(sequenceChunk);
    else
        prependChunk(chunk);
}

void SequenceChunk::appendChunk(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    const auto& mergedChunk = chunks.size() > 0 ? chunks.back()->merge(chunk) : nullptr;
    if (mergedChunk != nullptr)
        chunks.back() = mergedChunk;
    else
        chunks.push_back(chunk);
}

void SequenceChunk::appendChunk(const std::shared_ptr<SliceChunk>& sliceChunk)
{
    if (auto sequenceChunk = std::dynamic_pointer_cast<SequenceChunk>(sliceChunk->getChunk())) {
        int64_t byteOffset = 0;
        for (auto& elementChunk : sequenceChunk->chunks) {
            int64_t sliceChunkBegin = sliceChunk->getByteOffset();
            int64_t sliceChunkEnd = sliceChunk->getByteOffset() + sliceChunk->getByteLength();
            int64_t chunkBegin = byteOffset;
            int64_t chunkEnd = byteOffset + elementChunk->getByteLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                appendChunk(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                appendChunk(std::make_shared<SliceChunk>(elementChunk, sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                appendChunk(std::make_shared<SliceChunk>(elementChunk, 0, chunkEnd - sliceChunkEnd));
            byteOffset += elementChunk->getByteLength();
        }
    }
    else
        appendChunk(std::static_pointer_cast<Chunk>(sliceChunk));
}

void SequenceChunk::appendChunk(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto& chunk : chunk->chunks)
        appendChunk(chunk);
}

void SequenceChunk::append(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    if (!flatten)
        appendChunk(chunk);
    else if (auto sliceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        appendChunk(sliceChunk);
    else if (auto sequenceChunk = std::dynamic_pointer_cast<SliceChunk>(chunk))
        appendChunk(sequenceChunk);
    else
        appendChunk(chunk);
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
