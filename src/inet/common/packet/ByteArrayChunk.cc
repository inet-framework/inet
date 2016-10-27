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

void ByteArrayChunk::setBytes(const std::vector<uint8_t>& bytes)
{
    assertMutable();
    this->bytes = bytes;
}

void ByteArrayChunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    ByteOutputStream outputStream;
    chunk->serialize(outputStream);
    std::vector<uint8_t> chunkBytes;
    int byteCount = byteLength == -1 ? outputStream.getSize() : byteLength;
    for (int64_t i = 0; i < byteCount; i++)
        chunkBytes.push_back(outputStream[byteOffset + i]);
    setBytes(chunkBytes);
}

std::shared_ptr<Chunk> ByteArrayChunk::merge(const std::shared_ptr<Chunk>& other) const
{
    if (std::dynamic_pointer_cast<ByteArrayChunk>(other) != nullptr) {
        auto otherByteArrayChunk = std::static_pointer_cast<ByteArrayChunk>(other);
        std::vector<uint8_t> mergedBytes;
        mergedBytes.insert(mergedBytes.end(), bytes.begin(), bytes.end());
        mergedBytes.insert(mergedBytes.end(), otherByteArrayChunk->bytes.begin(), otherByteArrayChunk->bytes.end());
        return std::make_shared<ByteArrayChunk>(mergedBytes);
    }
    else
        return nullptr;
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
