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
    assert(this->offset >= 0);
    assert(this->length >= 0);
}

std::shared_ptr<Chunk> SliceChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    int64_t sliceChunkOffset = offset == -1 ? 0 : offset;
    int64_t sliceChunkLength = length == -1 ? chunk->getChunkLength() - sliceChunkOffset : length;
    assert(sliceChunkOffset >= 0);
    assert(sliceChunkLength >= 0);
    chunk->assertImmutable();
    return std::make_shared<SliceChunk>(chunk, sliceChunkOffset, sliceChunkLength);
}

void SliceChunk::setOffset(int64_t offset)
{
    assertMutable();
    assert(offset >= 0);
    this->offset = offset;
}

void SliceChunk::setLength(int64_t length)
{
    assertMutable();
    assert(length >= 0);
    this->length = length;
}

bool SliceChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        if (this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length) {
            offset -= otherSliceChunk->length;
            length += otherSliceChunk->length;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool SliceChunk::insertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        if (this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset) {
            length += otherSliceChunk->length;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool SliceChunk::removeFromBeginning(int64_t length)
{
    assert(length <= this->length);
    assertMutable();
    handleChange();
    this->offset += length;
    this->length -= length;
    return true;
}

bool SliceChunk::removeFromEnd(int64_t length)
{
    assert(length <= this->length);
    assertMutable();
    handleChange();
    this->length -= length;
    return true;
}

std::shared_ptr<Chunk> SliceChunk::peek(const Iterator& iterator, int64_t length) const
{
    if (iterator.getPosition() == 0 && length == getChunkLength())
        return const_cast<SliceChunk *>(this)->shared_from_this();
    else {
        Iterator sliceIterator(iterator);
        moveIterator(sliceIterator, offset);
        return chunk->peek(sliceIterator, length);
    }
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, chunk = {" << chunk << "}, offset = " << offset << ", length = " << length;
    return os.str();
}

} // namespace
