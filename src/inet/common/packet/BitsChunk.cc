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

std::shared_ptr<Chunk> BitsChunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    ByteOutputStream outputStream;
    Chunk::serialize(outputStream, chunk);
    std::vector<bool> bytes;
    bit chunkLength = chunk->getChunkLength();
    bit resultLength = length == bit(-1) ? chunkLength - offset : length;
    assert(bit(0) <= resultLength && resultLength <= chunkLength);
    for (bit i = bit(0); i < resultLength; i++)
        bytes.push_back(outputStream.getBit(bit(offset + i).get()));
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

void BitsChunk::removeFromBeginning(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    bits.erase(bits.begin(), bits.begin() + bit(length).get());
}

void BitsChunk::removeFromEnd(bit length)
{
    assert(bit(0) <= length && length <= getChunkLength());
    handleChange();
    bits.erase(bits.end() - bit(length).get(), bits.end());
}

std::shared_ptr<Chunk> BitsChunk::peek(const Iterator& iterator, bit length) const
{
    assert(bit(0) <= iterator.getPosition() && iterator.getPosition() <= getChunkLength());
    bit chunkLength = getChunkLength();
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1)))
        return nullptr;
    else if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength))
        return const_cast<BitsChunk *>(this)->shared_from_this();
    else {
        auto result = std::make_shared<BitsChunk>(std::vector<bool>(bits.begin() + bit(iterator.getPosition()).get(), length == bit(-1) ? bits.end() : bits.begin() + bit(iterator.getPosition() + length).get()));
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
