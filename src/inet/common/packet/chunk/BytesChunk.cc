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

std::shared_ptr<Chunk> BytesChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
{
    bit chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(bit(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1))) {
        if (predicate == nullptr || predicate(nullptr))
            return nullptr;
    }
    // 2. peeking the whole part returns this chunk
    if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength)) {
        auto result = const_cast<BytesChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a BytesChunk
    if (converter == nullptr) {
        auto chunk = std::make_shared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + byte(iterator.getPosition()).get(), length == bit(-1) ? bytes.end() : bytes.begin() + byte(iterator.getPosition() + length).get()));
        chunk->markImmutable();
        return chunk;
    }
    // 4. peeking with conversion
    return converter(const_cast<BytesChunk *>(this)->shared_from_this(), iterator, length, flags);
}

std::shared_ptr<Chunk> BytesChunk::convertChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length, int flags)
{
    ByteOutputStream outputStream((chunk->getChunkLength().get() + 7) >> 3);
    Chunk::serialize(outputStream, chunk, offset, length);
    const std::vector<uint8_t>& bytes = outputStream.getBytes();
    return std::make_shared<BytesChunk>(bytes);
}

void BytesChunk::setBytes(const std::vector<uint8_t>& bytes)
{
    handleChange();
    this->bytes = bytes;
}

void BytesChunk::setByte(int index, uint8_t byte)
{
    handleChange();
    bytes[index] = byte;
}

size_t BytesChunk::copyToBuffer(uint8_t *buffer, size_t bufferLength) const
{
    size_t length = bytes.size();
    CHUNK_CHECK_USAGE(buffer != nullptr, "buffer is nullptr");
    CHUNK_CHECK_USAGE(bufferLength >= length, "buffer is too small");
    std::copy(bytes.begin(), bytes.end(), buffer);
    return length;
}

void BytesChunk::copyFromBuffer(const uint8_t *buffer, size_t bufferLength)
{
    CHUNK_CHECK_USAGE(buffer != nullptr, "buffer is nullptr");
    handleChange();
    bytes.assign(buffer, buffer + bufferLength);
}

bool BytesChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == CT_BYTES;
}

bool BytesChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == CT_BYTES;
}

void BytesChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BYTES);
    handleChange();
    const auto& bytesChunk = std::static_pointer_cast<BytesChunk>(chunk);
    bytes.insert(bytes.begin(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BYTES);
    handleChange();
    const auto& bytesChunk = std::static_pointer_cast<BytesChunk>(chunk);
    bytes.insert(bytes.end(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::removeFromBeginning(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    bytes.erase(bytes.begin(), bytes.begin() + byte(length).get());
}

void BytesChunk::removeFromEnd(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    bytes.erase(bytes.end() - byte(length).get(), bytes.end());
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
