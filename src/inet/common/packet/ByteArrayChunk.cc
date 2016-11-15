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

#include "inet/common/packet/ByteArrayChunk.h"

namespace inet {

ByteArrayChunk::ByteArrayChunk(const std::vector<uint8_t>& bytes) :
    bytes(bytes)
{
}

std::shared_ptr<Chunk> ByteArrayChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, chunk);
    std::vector<uint8_t> chunkBytes;
    int byteCount = byteLength == -1 ? outputStream.getSize() : byteLength;
    for (int64_t i = 0; i < byteCount; i++)
        chunkBytes.push_back(outputStream[byteOffset + i]);
    return std::make_shared<ByteArrayChunk>(chunkBytes);
}

void ByteArrayChunk::setBytes(const std::vector<uint8_t>& bytes)
{
    assertMutable();
    this->bytes = bytes;
}

bool ByteArrayChunk::insertToBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_BYTEARRAY) {
        const auto& byteArrayChunk = std::static_pointer_cast<ByteArrayChunk>(chunk);
        bytes.insert(bytes.begin(), byteArrayChunk->bytes.begin(), byteArrayChunk->bytes.end());
        return true;
    }
    else
        return false;
}

bool ByteArrayChunk::insertToEnd(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    handleChange();
    if (chunk->getChunkType() == TYPE_BYTEARRAY) {
        const auto& byteArrayChunk = std::static_pointer_cast<ByteArrayChunk>(chunk);
        bytes.insert(bytes.end(), byteArrayChunk->bytes.begin(), byteArrayChunk->bytes.end());
        return true;
    }
    else
        return false;
}

bool ByteArrayChunk::removeFromBeginning(int64_t byteLength)
{
    assert(byteLength <= bytes.size());
    assertMutable();
    handleChange();
    bytes.erase(bytes.begin(), bytes.begin() + byteLength);
    return true;
}

bool ByteArrayChunk::removeFromEnd(int64_t byteLength)
{
    assert(byteLength <= bytes.size());
    assertMutable();
    handleChange();
    bytes.erase(bytes.end() - byteLength, bytes.end());
    return true;
}

std::shared_ptr<Chunk> ByteArrayChunk::peek(const Iterator& iterator, int64_t byteLength) const
{
    if (iterator.getPosition() == 0 && byteLength == getByteLength())
        return const_cast<ByteArrayChunk *>(this)->shared_from_this();
    else
        return std::make_shared<ByteArrayChunk>(std::vector<uint8_t>(bytes.begin() + iterator.getPosition(), byteLength == -1 ? bytes.end() : bytes.begin() + iterator.getPosition() + byteLength));
}

std::string ByteArrayChunk::str() const
{
    std::ostringstream os;
    os << "ByteArrayChunk, byteLength = " << getByteLength() << ", bytes = {";
    bool first = true;
    for (auto byte : bytes) {
        if (!first)
            os << ", ";
        else
            first = false;

        os << (int) (byte);
    }
    os << "}";
    return os.str();
}

} // namespace
