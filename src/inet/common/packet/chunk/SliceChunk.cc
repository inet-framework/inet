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

SliceChunk::SliceChunk() :
    Chunk(),
    chunk(nullptr),
    offset(-1),
    length(-1)
{
}

SliceChunk::SliceChunk(const SliceChunk& other) :
    Chunk(other),
    chunk(other.chunk),
    offset(other.offset),
    length(other.length)
{
}

SliceChunk::SliceChunk(const std::shared_ptr<Chunk>& chunk, bit offset, bit length) :
    Chunk(),
    chunk(chunk),
    offset(offset),
    length(length == bit(-1) ? chunk->getChunkLength() - offset : length)
{
    assert(chunk->isImmutable());
    bit chunkLength = chunk->getChunkLength();
    assert(bit(0) <= this->offset && this->offset <= chunkLength);
    assert(bit(0) <= this->length && this->offset + this->length <= chunkLength);
}

std::shared_ptr<Chunk> SliceChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
{
    bit chunkLength = getChunkLength();
    assert(bit(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength);
    // 1. peeking an empty part returns nullptr
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1))) {
        if (predicate == nullptr || predicate(nullptr))
            return nullptr;
    }
    // 2. peeking the whole part
    if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength)) {
        // 2.1 peeking the whole part returns the sliced chunk
        if (offset == bit(0) && chunkLength == chunk->getChunkLength()) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
        // 2.2 peeking the whole part returns this chunk
        auto result = const_cast<SliceChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking anything else returns what peeking the sliced chunk returns
    return chunk->peekUnchecked(predicate, converter, ForwardIterator(iterator.getPosition() + offset, -1), length == bit(-1) ? chunkLength : length, flags);
}

std::shared_ptr<Chunk> SliceChunk::convertChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    assert(chunk->isImmutable());
    bit chunkLength = chunk->getChunkLength();
    bit sliceLength = length == bit(-1) ? chunkLength - offset : length;
    assert(bit(0) <= offset && offset <= chunkLength);
    assert(bit(0) <= sliceLength && sliceLength <= chunkLength);
    return std::make_shared<SliceChunk>(chunk, offset, sliceLength);
}

void SliceChunk::setOffset(bit offset)
{
    assert(isMutable());
    assert(bit(0) <= offset && offset <= chunk->getChunkLength());
    this->offset = offset;
}

void SliceChunk::setLength(bit length)
{
    assert(isMutable());
    assert(bit(0) <= length && length <= chunk->getChunkLength());
    this->length = length;
}

bool SliceChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    if (chunk->getChunkType() == CT_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length;
    }
    else
        return false;
}

bool SliceChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    if (chunk->getChunkType() == CT_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset;
    }
    else
        return false;
}

void SliceChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == CT_SLICE);
    handleChange();
    const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
    assert(this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length);
    offset -= otherSliceChunk->length;
    length += otherSliceChunk->length;
}

void SliceChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == CT_SLICE);
    handleChange();
    const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
    assert(this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset);
    length += otherSliceChunk->length;
}

void SliceChunk::removeFromBeginning(bit length)
{
    assert(bit(0) <= length && length <= this->length);
    handleChange();
    this->offset += length;
    this->length -= length;
}

void SliceChunk::removeFromEnd(bit length)
{
    assert(bit(0) <= length && length <= this->length);
    handleChange();
    this->length -= length;
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, offset = " << offset << ", length = " << length << ", chunk = {" << chunk << "}";
    return os.str();
}

} // namespace
