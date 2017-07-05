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

#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

ByteCountChunk::ByteCountChunk() :
    Chunk(),
    length(-1),
    data('?')
{
}

ByteCountChunk::ByteCountChunk(const ByteCountChunk& other) :
    Chunk(other),
    length(other.length),
    data(other.data)
{
}

ByteCountChunk::ByteCountChunk(byte length, uint8_t data) :
    Chunk(),
    length(length),
    data(data)
{
    CHUNK_CHECK_USAGE(length >= byte(0), "length is invalid");
}

Ptr<Chunk> ByteCountChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
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
        auto result = const_cast<ByteCountChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a ByteCountChunk
    if (converter == nullptr) {
        auto chunk = std::make_shared<ByteCountChunk>(length == bit(-1) ? chunkLength - iterator.getPosition() : length);
        chunk->markImmutable();
        return chunk;
    }
    // 4. peeking with conversion
    return converter(const_cast<ByteCountChunk *>(this)->shared_from_this(), iterator, length, flags);
}

Ptr<Chunk> ByteCountChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, bit offset, bit length, int flags)
{
    bit chunkLength = chunk->getChunkLength();
    bit resultLength = length == bit(-1) ? chunkLength - offset : length;
    CHUNK_CHECK_IMPLEMENTATION(bit(0) <= resultLength && resultLength <= chunkLength);
    return std::make_shared<ByteCountChunk>(byte(resultLength));
}

void ByteCountChunk::setLength(byte length)
{
    CHUNK_CHECK_USAGE(length >= byte(0), "length is invalid");
    handleChange();
    this->length = length;
}

void ByteCountChunk::setData(uint8_t data)
{
    handleChange();
    this->data = data;
}

bool ByteCountChunk::canInsertAtBeginning(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTECOUNT;
}

bool ByteCountChunk::canInsertAtEnd(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTECOUNT;
}

void ByteCountChunk::insertAtBeginning(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BYTECOUNT);
    handleChange();
    const auto& byteCountChunk = std::static_pointer_cast<const ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::insertAtEnd(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BYTECOUNT);
    handleChange();
    const auto& byteCountChunk = std::static_pointer_cast<const ByteCountChunk>(chunk);
    length += byteCountChunk->length;
}

void ByteCountChunk::removeFromBeginning(bit length)
{
    CHUNK_CHECK_IMPLEMENTATION(bit(0) <= length && length <= getChunkLength());
    handleChange();
    this->length -= byte(length);
}

void ByteCountChunk::removeFromEnd(bit length)
{
    CHUNK_CHECK_IMPLEMENTATION(bit(0) <= length && length <= getChunkLength());
    handleChange();
    this->length -= byte(length);
}

std::string ByteCountChunk::str() const
{
    std::ostringstream os;
    os << "ByteCountChunk, length = " << byte(getChunkLength());
    return os.str();
}

} // namespace
