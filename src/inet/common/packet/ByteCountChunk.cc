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

#include "inet/common/packet/ByteCountChunk.h"

namespace inet {

ByteCountChunk::ByteCountChunk() :
    Chunk(),
    length(-1)
{
}

ByteCountChunk::ByteCountChunk(const ByteCountChunk& other) :
    Chunk(other),
    length(other.length)
{
}

ByteCountChunk::ByteCountChunk(int64_t length) :
    Chunk(),
    length(length)
{
    assert(length >= 0);
}

std::shared_ptr<Chunk> ByteCountChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    int64_t chunkLength = chunk->getChunkLength();
    int64_t resultLength = length == -1 ? chunkLength - offset : length;
    assert(0 <= resultLength && resultLength <= chunkLength);
    assert(resultLength % 8 == 0);
    return std::make_shared<ByteCountChunk>(resultLength / 8);
}

void ByteCountChunk::setLength(int64_t length)
{
    assertMutable();
    assert(length >= 0);
    this->length = length;
}

bool ByteCountChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BYTECOUNT;
}

bool ByteCountChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BYTECOUNT;
}

void ByteCountChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BYTECOUNT);
    handleChange();
    const auto& byteCountChunk = std::static_pointer_cast<ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BYTECOUNT);
    handleChange();
    const auto& byteCountChunk = std::static_pointer_cast<ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= getChunkLength());
    assert(length % 8 == 0);
    handleChange();
    this->length -= length / 8;
}

void ByteCountChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= getChunkLength());
    assert(length % 8 == 0);
    handleChange();
    this->length -= length / 8;
}

std::shared_ptr<Chunk> ByteCountChunk::peek(const Iterator& iterator, int64_t length) const
{
    assert(0 <= iterator.getPosition() && iterator.getPosition() <= getChunkLength());
    int64_t chunkLength = getChunkLength();
    if (length == 0 || (iterator.getPosition() == chunkLength && length == -1))
        return nullptr;
    else if (iterator.getPosition() == 0 && (length == -1 || length == chunkLength))
        return const_cast<ByteCountChunk *>(this)->shared_from_this();
    else {
        int64_t resultLength = length == -1 ? chunkLength - iterator.getPosition() : length;
        assert(resultLength % 8 == 0);
        auto result = std::make_shared<ByteCountChunk>(resultLength / 8);
        result->markImmutable();
        return result;
    }
}

std::string ByteCountChunk::str() const
{
    std::ostringstream os;
    os << "ByteCountChunk, length = " << getChunkLength();
    return os.str();
}

} // namespace
