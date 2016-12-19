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

#include "inet/common/packet/BytesChunk.h"

namespace inet {

BytesChunk::BytesChunk() :
    Chunk()
{
}

BytesChunk::BytesChunk(const BytesChunk& other) :
    Chunk(other),
    bytes(other.bytes)
{
}

BytesChunk::BytesChunk(const std::vector<uint8_t>& bytes) :
    Chunk(),
    bytes(bytes)
{
}

std::shared_ptr<Chunk> BytesChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, chunk);
    std::vector<uint8_t> bytes;
    int64_t chunkLength = chunk->getChunkLength();
    int64_t resultLength = length == -1 ? chunkLength - offset : length;
    assert(0 <= resultLength && resultLength <= chunkLength);
    for (int64_t i = 0; i < resultLength; i++)
        bytes.push_back(outputStream[offset + i]);
    return std::make_shared<BytesChunk>(bytes);
}

void BytesChunk::setBytes(const std::vector<uint8_t>& bytes)
{
    assertMutable();
    this->bytes = bytes;
}

void BytesChunk::setByte(int index, uint8_t byte)
{
    assertMutable();
    bytes[index] = byte;
}

bool BytesChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BYTES;
}

bool BytesChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BYTES;
}

void BytesChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BYTES);
    handleChange();
    const auto& bytesChunk = std::static_pointer_cast<BytesChunk>(chunk);
    bytes.insert(bytes.begin(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BYTES);
    handleChange();
    const auto& bytesChunk = std::static_pointer_cast<BytesChunk>(chunk);
    bytes.insert(bytes.end(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= bytes.size());
    handleChange();
    bytes.erase(bytes.begin(), bytes.begin() + length);
}

void BytesChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= bytes.size());
    handleChange();
    bytes.erase(bytes.end() - length, bytes.end());
}

std::shared_ptr<Chunk> BytesChunk::peek(const Iterator& iterator, int64_t length) const
{
    assert(0 <= iterator.getPosition() && iterator.getPosition() <= bytes.size());
    if (length == 0 || (iterator.getPosition() == bytes.size() && length == -1))
        return nullptr;
    else if (iterator.getPosition() == 0 && (length == -1 || length == bytes.size()))
        return const_cast<BytesChunk *>(this)->shared_from_this();
    else {
        assertImmutable();
        auto result = std::make_shared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + iterator.getPosition(), length == -1 ? bytes.end() : bytes.begin() + iterator.getPosition() + length));
        result->markImmutable();
        return result;
    }
}

std::string BytesChunk::str() const
{
    std::ostringstream os;
    os << "BytesChunk, length = " << bytes.size() << ", bytes = {";
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
