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

#include "inet/common/packet/chunk/BytesChunk.h"

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

std::shared_ptr<Chunk> BytesChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    ByteOutputStream outputStream((chunk->getChunkLength().get() + 7) >> 3);
    Chunk::serialize(outputStream, chunk, offset, length);
    const std::vector<uint8_t>& bytes = outputStream.getBytes();
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

size_t BytesChunk::getBytes(uint8_t *buffer, size_t bufLen) const
{
    size_t numBytes = bytes.size();
    if (bufLen < numBytes)
        throw cRuntimeError("getBytes(): buffer is too short");
    for (size_t i = 0; i < numBytes; i++)
        buffer[i] = bytes[i];
    return numBytes;
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

void BytesChunk::removeFromBeginning(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    bytes.erase(bytes.begin(), bytes.begin() + byte(length).get());
}

void BytesChunk::removeFromEnd(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    bytes.erase(bytes.end() - byte(length).get(), bytes.end());
}

std::shared_ptr<Chunk> BytesChunk::peekUnchecked(const Iterator& iterator, bit length) const
{
    assert(bit(0) <= iterator.getPosition() && iterator.getPosition() <= getChunkLength());
    bit chunkLength = getChunkLength();
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1)))
        return nullptr;
    else if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength))
        return const_cast<BytesChunk *>(this)->shared_from_this();
    else {
        auto result = std::make_shared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + byte(iterator.getPosition()).get(), length == bit(-1) ? bytes.end() : bytes.begin() + byte(iterator.getPosition() + length).get()));
        result->markImmutable();
        return result;
    }
}

std::string BytesChunk::str() const
{
    std::ostringstream os;
    os << "BytesChunk, length = " << byte(getChunkLength()) << ", bytes = {";
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
