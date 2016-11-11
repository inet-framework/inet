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

#include "inet/common/packet/ByteLengthChunk.h"

namespace inet {

ByteLengthChunk::ByteLengthChunk(int64_t byteLength) :
    byteLength(byteLength)
{
    assert(byteLength >= 0);
}

std::shared_ptr<Chunk> ByteLengthChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    return std::make_shared<ByteLengthChunk>(byteLength);
}

void ByteLengthChunk::setByteLength(int64_t byteLength)
{
    assertMutable();
    assert(byteLength >= 0);
    this->byteLength = byteLength;
}

bool ByteLengthChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    if (const auto& byteLengthChunk = std::dynamic_pointer_cast<ByteLengthChunk>(chunk)) {
        byteLength += byteLengthChunk->byteLength;
        return true;
    }
    else
        return false;
}

bool ByteLengthChunk::insertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    if (const auto& byteLengthChunk = std::dynamic_pointer_cast<ByteLengthChunk>(chunk)) {
        byteLength += byteLengthChunk->byteLength;
        return true;
    }
    else
        return false;
}

bool ByteLengthChunk::removeFromBeginning(int64_t byteLength)
{
    assert(byteLength <= this->byteLength);
    this->byteLength -= byteLength;
    return true;
}

bool ByteLengthChunk::removeFromEnd(int64_t byteLength)
{
    assert(byteLength <= this->byteLength);
    this->byteLength -= byteLength;
    return true;
}

std::shared_ptr<Chunk> ByteLengthChunk::peek(const Iterator& iterator, int64_t byteLength) const
{
    if (iterator.getPosition() == 0 && byteLength == getByteLength())
        return const_cast<ByteLengthChunk *>(this)->shared_from_this();
    else
        return std::make_shared<ByteLengthChunk>(byteLength);
}

std::string ByteLengthChunk::str() const
{
    std::ostringstream os;
    os << "ByteLengthChunk, byteLength = " << byteLength;
    return os.str();
}

} // namespace
