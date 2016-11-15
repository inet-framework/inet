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
    byteLength(-1)
{
}

SliceChunk::SliceChunk(const SliceChunk& other) :
    Chunk(other),
    chunk(other.chunk),
    byteOffset(other.byteOffset),
    byteLength(other.byteLength)
{
}

SliceChunk::SliceChunk(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength) :
    Chunk(),
    chunk(chunk),
    byteOffset(byteOffset),
    byteLength(byteLength == -1 ? chunk->getByteLength() - byteOffset : byteLength)
{
    chunk->assertImmutable();
    assert(this->byteOffset >= 0);
    assert(this->byteLength >= 0);
}

std::shared_ptr<Chunk> SliceChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    int64_t sliceChunkByteOffset = byteOffset == -1 ? 0 : byteOffset;
    int64_t sliceChunkByteLength = byteLength == -1 ? chunk->getByteLength() - sliceChunkByteOffset : byteLength;
    assert(sliceChunkByteOffset >= 0);
    assert(sliceChunkByteLength >= 0);
    chunk->assertImmutable();
    return std::make_shared<SliceChunk>(chunk, sliceChunkByteOffset, sliceChunkByteLength);
}

void SliceChunk::setByteOffset(int64_t byteOffset)
{
    assertMutable();
    assert(byteOffset >= 0);
    this->byteOffset = byteOffset;
}

void SliceChunk::setByteLength(int64_t byteLength)
{
    assertMutable();
    assert(byteLength >= 0);
    this->byteLength = byteLength;
}

bool SliceChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_SLICE) {
        const auto& otherSliceChunk = std::static_pointer_cast<SliceChunk>(chunk);
        if (this->chunk == otherSliceChunk->chunk && byteOffset == otherSliceChunk->byteOffset + otherSliceChunk->byteLength) {
            byteOffset -= otherSliceChunk->byteLength;
            byteLength += otherSliceChunk->byteLength;
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
        if (this->chunk == otherSliceChunk->chunk && byteOffset + byteLength == otherSliceChunk->byteOffset) {
            byteLength += otherSliceChunk->byteLength;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool SliceChunk::removeFromBeginning(int64_t byteLength)
{
    assert(byteLength <= this->byteLength);
    assertMutable();
    handleChange();
    this->byteOffset += byteLength;
    this->byteLength -= byteLength;
    return true;
}

bool SliceChunk::removeFromEnd(int64_t byteLength)
{
    assert(byteLength <= this->byteLength);
    assertMutable();
    handleChange();
    this->byteLength -= byteLength;
    return true;
}

std::shared_ptr<Chunk> SliceChunk::peek(const Iterator& iterator, int64_t byteLength) const
{
    if (iterator.getPosition() == 0 && byteLength == getByteLength())
        return const_cast<SliceChunk *>(this)->shared_from_this();
    else {
        Iterator sliceIterator(iterator);
        sliceIterator.move(shared_from_this(), byteOffset);
        return chunk->peek(sliceIterator, byteLength);
    }
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, chunk = {" << chunk << "}, byteOffset = " << byteOffset << ", byteLength = " << byteLength;
    return os.str();
}

} // namespace
