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

#include "inet/common/packet/LengthChunk.h"

namespace inet {

LengthChunk::LengthChunk() :
    Chunk(),
    length(-1)
{
}

LengthChunk::LengthChunk(const LengthChunk& other) :
    Chunk(other),
    length(other.length)
{
}

LengthChunk::LengthChunk(int64_t length) :
    Chunk(),
    length(length)
{
    assert(length >= 0);
}

std::shared_ptr<Chunk> LengthChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t length)
{
    return std::make_shared<LengthChunk>(length == -1 ? chunk->getChunkLength() : length);
}

void LengthChunk::setByteLength(int64_t length)
{
    assertMutable();
    assert(length >= 0);
    this->length = length;
}

bool LengthChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_LENGTH) {
        const auto& lengthChunk = std::static_pointer_cast<LengthChunk>(chunk);
        length += lengthChunk->length;
        return true;
    }
    else
        return false;
}

bool LengthChunk::insertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_LENGTH) {
        const auto& lengthChunk = std::static_pointer_cast<LengthChunk>(chunk);
        length += lengthChunk->length;
        return true;
    }
    else
        return false;
}

bool LengthChunk::removeFromBeginning(int64_t length)
{
    assert(length <= this->length);
    assertMutable();
    handleChange();
    this->length -= length;
    return true;
}

bool LengthChunk::removeFromEnd(int64_t length)
{
    assert(length <= this->length);
    assertMutable();
    handleChange();
    this->length -= length;
    return true;
}

std::shared_ptr<Chunk> LengthChunk::peek(const Iterator& iterator, int64_t length) const
{
    if (iterator.getPosition() == 0 && length == getChunkLength())
        return const_cast<LengthChunk *>(this)->shared_from_this();
    else
        return std::make_shared<LengthChunk>(length == -1 ? getChunkLength() - iterator.getPosition() : length);
}

std::string LengthChunk::str() const
{
    std::ostringstream os;
    os << "LengthChunk, length = " << length;
    return os.str();
}

} // namespace
