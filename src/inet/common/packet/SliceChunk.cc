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
    byteOffset(-1),
    length(-1)
{
}

SliceChunk::SliceChunk(const SliceChunk& other) :
    Chunk(other),
    chunk(other.chunk),
    byteOffset(other.byteOffset),
    length(other.length)
{
}

SliceChunk::SliceChunk(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t length) :
    Chunk(),
    chunk(chunk),
    byteOffset(byteOffset),
    length(length == -1 ? chunk->getChunkLength() - byteOffset : length)
{
    chunk->assertImmutable();
    assert(this->byteOffset >= 0);
    assert(this->length >= 0);
}

std::shared_ptr<Chunk> SliceChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t length)
{
    int64_t sliceChunkByteOffset = byteOffset == -1 ? 0 : byteOffset;
    int64_t sliceChunkLength = length == -1 ? chunk->getChunkLength() - sliceChunkByteOffset : length;
    assert(sliceChunkByteOffset >= 0);
    assert(sliceChunkLength >= 0);
    chunk->assertImmutable();
    return std::make_shared<SliceChunk>(chunk, sliceChunkByteOffset, sliceChunkLength);
}

void SliceChunk::setByteOffset(int64_t byteOffset)
{
    assertMutable();
    assert(byteOffset >= 0);
    this->byteOffset = byteOffset;
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
        if (this->chunk == otherSliceChunk->chunk && byteOffset == otherSliceChunk->byteOffset + otherSliceChunk->length) {
            byteOffset -= otherSliceChunk->length;
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
        if (this->chunk == otherSliceChunk->chunk && byteOffset + length == otherSliceChunk->byteOffset) {
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
    this->byteOffset += length;
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
        moveIterator(sliceIterator, byteOffset);
        return chunk->peek(sliceIterator, length);
    }
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, chunk = {" << chunk << "}, byteOffset = " << byteOffset << ", length = " << length;
    return os.str();
}

} // namespace
