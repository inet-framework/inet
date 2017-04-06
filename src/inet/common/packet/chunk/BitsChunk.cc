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

#include "inet/common/packet/chunk/BitsChunk.h"

namespace inet {

BitsChunk::BitsChunk() :
    Chunk()
{
}

BitsChunk::BitsChunk(const BitsChunk& other) :
    Chunk(other),
    bits(other.bits)
{
}

BitsChunk::BitsChunk(const std::vector<bool>& bits) :
    Chunk(),
    bits(bits)
{
}

std::shared_ptr<Chunk> BitsChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
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
        auto result = const_cast<BitsChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a BitsChunk
    if (converter == nullptr) {
        auto result = std::make_shared<BitsChunk>(std::vector<bool>(bits.begin() + bit(iterator.getPosition()).get(), length == bit(-1) ? bits.end() : bits.begin() + bit(iterator.getPosition() + length).get()));
        result->markImmutable();
        return result;
    }
    // 4. peeking with conversion
    return converter(const_cast<BitsChunk *>(this)->shared_from_this(), iterator, length, flags);
}

std::shared_ptr<Chunk> BitsChunk::convertChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length, int flags)
{
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, chunk);
    std::vector<bool> bits;
    bit chunkLength = chunk->getChunkLength();
    bit resultLength = length == bit(-1) ? chunkLength - offset : length;
    CHUNK_CHECK_IMPLEMENTATION(bit(0) <= resultLength && resultLength <= chunkLength);
    outputStream.copyBits(bits, bit(offset).get(), bit(resultLength).get());
    return std::make_shared<BitsChunk>(bits);
}

void BitsChunk::setBits(const std::vector<bool>& bits)
{
    handleChange();
    this->bits = bits;
}

void BitsChunk::setBit(int index, bool bit)
{
    handleChange();
    bits[index] = bit;
}

bool BitsChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == CT_BITS;
}

bool BitsChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == CT_BITS;
}

void BitsChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BITS);
    handleChange();
    const auto& bitsChunk = std::static_pointer_cast<BitsChunk>(chunk);
    bits.insert(bits.begin(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkType() == CT_BITS);
    handleChange();
    const auto& bitsChunk = std::static_pointer_cast<BitsChunk>(chunk);
    bits.insert(bits.end(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::removeFromBeginning(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    bits.erase(bits.begin(), bits.begin() + bit(length).get());
}

void BitsChunk::removeFromEnd(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getChunkLength(), "length is invalid");
    handleChange();
    bits.erase(bits.end() - bit(length).get(), bits.end());
}

std::string BitsChunk::str() const
{
    std::ostringstream os;
    os << "BitsChunk, length = " << bits.size() << ", bits = {";
    for (auto bit : bits)
        os << (bit ? "1" : "0");
    os << "}";
    return os.str();
}

} // namespace
