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

#include "inet/common/packet/BitsChunk.h"

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

std::shared_ptr<Chunk> BitsChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, chunk);
    std::vector<bool> bytes;
    int64_t chunkLength = chunk->getChunkLength();
    int64_t resultLength = length == -1 ? chunkLength - offset : length;
    assert(0 <= resultLength && resultLength <= chunkLength);
    for (int64_t i = 0; i < resultLength; i++)
        bytes.push_back(outputStream[offset + i]);
    return std::make_shared<BitsChunk>(bytes);
}

void BitsChunk::setBits(const std::vector<bool>& bits)
{
    assertMutable();
    this->bits = bits;
}

void BitsChunk::setBit(int index, bool bit)
{
    assertMutable();
    bits[index] = bit;
}

bool BitsChunk::canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BITS;
}

bool BitsChunk::canInsertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    return chunk->getChunkType() == TYPE_BITS;
}

void BitsChunk::insertAtBeginning(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BITS);
    handleChange();
    const auto& bitsChunk = std::static_pointer_cast<BitsChunk>(chunk);
    bits.insert(bits.begin(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::insertAtEnd(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk->getChunkType() == TYPE_BITS);
    handleChange();
    const auto& bitsChunk = std::static_pointer_cast<BitsChunk>(chunk);
    bits.insert(bits.end(), bitsChunk->bits.begin(), bitsChunk->bits.end());
}

void BitsChunk::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= bits.size());
    handleChange();
    bits.erase(bits.begin(), bits.begin() + length);
}

void BitsChunk::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= bits.size());
    handleChange();
    bits.erase(bits.end() - length, bits.end());
}

std::shared_ptr<Chunk> BitsChunk::peek(const Iterator& iterator, int64_t length) const
{
    assert(0 <= iterator.getPosition() && iterator.getPosition() <= getChunkLength());
    int64_t chunkLength = getChunkLength();
    if (length == 0 || (iterator.getPosition() == chunkLength && length == -1))
        return nullptr;
    else if (iterator.getPosition() == 0 && (length == -1 || length == chunkLength))
        return const_cast<BitsChunk *>(this)->shared_from_this();
    else {
        auto result = std::make_shared<BitsChunk>(std::vector<bool>(bits.begin() + iterator.getPosition(), length == -1 ? bits.end() : bits.begin() + iterator.getPosition() + length));
        result->markImmutable();
        return result;
    }
}

std::string BitsChunk::str() const
{
    std::ostringstream os;
    os << "BitsChunk, length = " << bits.size() << ", bits = {";
    for (auto bit : bits)
        if (bit)
            os << "1";
        else
            os << "0";
    os << "}";
    return os.str();
}

} // namespace
