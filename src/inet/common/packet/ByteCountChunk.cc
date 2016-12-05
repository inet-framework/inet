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
    return std::make_shared<ByteCountChunk>(resultLength);
}

void ByteCountChunk::setLength(int64_t length)
{
    assertMutable();
    assert(length >= 0);
    this->length = length;
}

bool ByteCountChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == TYPE_LENGTH) {
        const auto& byteCountChunk = std::static_pointer_cast<ByteCountChunk>(chunk);
        length += byteCountChunk->length;
        return true;
    }
    else
        return false;
}

bool ByteCountChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    handleChange();
    if (chunk->getChunkType() == TYPE_LENGTH) {
        const auto& byteCountChunk = std::static_pointer_cast<ByteCountChunk>(chunk);
        length += byteCountChunk->length;
        return true;
    }
    else
        return false;
}

bool ByteCountChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= this->length);
    handleChange();
    this->length -= length;
    return true;
}

bool ByteCountChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= this->length);
    handleChange();
    this->length -= length;
    return true;
}

std::shared_ptr<Chunk> ByteCountChunk::peek(const Iterator& iterator, int64_t length) const
{
    if (iterator.getPosition() == 0 && (length == -1 || length == getChunkLength()))
        return const_cast<ByteCountChunk *>(this)->shared_from_this();
    else
        return std::make_shared<ByteCountChunk>(length == -1 ? getChunkLength() - iterator.getPosition() : length);
}

std::string ByteCountChunk::str() const
{
    std::ostringstream os;
    os << "ByteCountChunk, length = " << length;
    return os.str();
}

} // namespace
