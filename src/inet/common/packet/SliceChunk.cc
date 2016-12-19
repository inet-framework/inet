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

SliceChunk::SliceChunk(const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length) :
    Chunk(),
    chunk(chunk),
    offset(offset),
    length(length == -1 ? chunk->getChunkLength() - offset : length)
{
    chunk->assertImmutable();
    int64_t chunkLength = chunk->getChunkLength();
    assert(0 <= this->offset && this->offset <= chunkLength);
    assert(0 <= this->length && this->offset + this->length <= chunkLength);
}

std::shared_ptr<Chunk> SliceChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    chunk->assertImmutable();
    int64_t chunkLength = chunk->getChunkLength();
    int64_t sliceLength = length == -1 ? chunkLength - offset : length;
    assert(0 <= offset && offset <= chunkLength);
    assert(0 <= sliceLength && sliceLength <= chunkLength);
    return std::make_shared<SliceChunk>(chunk, offset, sliceLength);
}

std::shared_ptr<Chunk> SliceChunk::peekSliceChunk(const Iterator& iterator, int64_t length) const
{
    if (iterator.getPosition() == 0 && (length == -1 || length == this->length)) {
        if (offset == 0 && this->length == chunk->getChunkLength())
            return chunk;
        else
            return const_cast<SliceChunk *>(this)->shared_from_this();
    }
    else
        return chunk->peek(ForwardIterator(iterator.getPosition() + offset, -1), length);
}

void SliceChunk::setOffset(int64_t offset)
{
    assertMutable();
    assert(0 <= offset && offset <= chunk->getChunkLength());
    this->offset = offset;
}

void SliceChunk::setLength(int64_t length)
{
    assertMutable();
    assert(0 <= length && length <= chunk->getChunkLength());
    this->length = length;
}

bool SliceChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    if (chunk->getChunkType() == TYPE_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length;
    }
    else
        return false;
}

bool SliceChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    if (chunk->getChunkType() == TYPE_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset;
    }
    else
        return false;
}

void SliceChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_SLICE);
    handleChange();
    const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
    assert(this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length);
    offset -= otherSliceChunk->length;
    length += otherSliceChunk->length;
}

void SliceChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_SLICE);
    handleChange();
    const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
    assert(this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset);
    length += otherSliceChunk->length;
}

bool SliceChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= this->length);
    handleChange();
    this->offset += length;
    this->length -= length;
    return true;
}

bool SliceChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= this->length);
    handleChange();
    this->length -= length;
    return true;
}

std::shared_ptr<Chunk> SliceChunk::peek(const Iterator& iterator, int64_t length) const
{
    assert(0 <= iterator.getPosition() && iterator.getPosition() <= this->length);
    if (length == 0 || (iterator.getPosition() == this->length && length == -1))
        return nullptr;
    else
        return peekSliceChunk(iterator, length);
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, chunk = {" << chunk << "}, offset = " << offset << ", length = " << length;
    return os.str();
}

} // namespace
