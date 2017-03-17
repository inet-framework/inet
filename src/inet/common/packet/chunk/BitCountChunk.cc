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

#include "inet/common/packet/chunk/BitCountChunk.h"

namespace inet {

BitCountChunk::BitCountChunk() :
    Chunk(),
    length(-1)
{
}

BitCountChunk::BitCountChunk(const BitCountChunk& other) :
    Chunk(other),
    length(other.length)
{
}

BitCountChunk::BitCountChunk(bit length) :
    Chunk(),
    length(length)
{
    assert(length >= bit(0));
}

std::shared_ptr<Chunk> BitCountChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    bit chunkLength = chunk->getChunkLength();
    bit resultLength = length == bit(-1) ? chunkLength - offset : length;
    assert(bit(0) <= resultLength && resultLength <= chunkLength);
    return std::make_shared<BitCountChunk>(resultLength);
}

void BitCountChunk::setLength(bit length)
{
    assertMutable();
    assert(length >= bit(0));
    this->length = length;
}

bool BitCountChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BITCOUNT;
}

bool BitCountChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BITCOUNT;
}

void BitCountChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BITCOUNT);
    handleChange();
    const auto& bitCountChunk = std::static_pointer_cast<BitCountChunk>(chunk);
    length += bitCountChunk->length;
}

void BitCountChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BITCOUNT);
    handleChange();
    const auto& bitCountChunk = std::static_pointer_cast<BitCountChunk>(chunk);
    length += bitCountChunk->length;
}

void BitCountChunk::removeFromBeginning(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    this->length -= length;
}

void BitCountChunk::removeFromEnd(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    this->length -= length;
}

std::shared_ptr<Chunk> BitCountChunk::peek(const Iterator& iterator, bit length, int flags) const
{
    assert(bit(0) <= iterator.getPosition() && iterator.getPosition() <= getChunkLength());
    bit chunkLength = getChunkLength();
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1)))
        return nullptr;
    else if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength))
        return const_cast<BitCountChunk *>(this)->shared_from_this();
    else {
        auto result = std::make_shared<BitCountChunk>(length == bit(-1) ? getChunkLength() - iterator.getPosition() : length);
        result->markImmutable();
        return result;
    }
}

std::string BitCountChunk::str() const
{
    std::ostringstream os;
    os << "BitCountChunk, length = " << length;
    return os.str();
}

} // namespace
