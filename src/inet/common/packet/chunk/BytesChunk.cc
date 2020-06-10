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
#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

Register_Class(BytesChunk);

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

void BytesChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(bytes.size());
    buffer->pack(bytes.data(), bytes.size());
}

void BytesChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    size_t size;
    buffer->unpack(size);
    bytes.resize(size);
    buffer->unpack(bytes.data(), bytes.size());
}

const Ptr<Chunk> BytesChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    b chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(b(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == b(0) || (iterator.getPosition() == chunkLength && length < b(0))) {
        if (predicate == nullptr || predicate(nullptr))
            return EmptyChunk::getEmptyChunk(flags);
    }
    // 2. peeking the whole part returns this chunk
    if (iterator.getPosition() == b(0) && (-length >= chunkLength || length == chunkLength)) {
        auto result = const_cast<BytesChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a BytesChunk or SliceChunk
    if (converter == nullptr) {
        // 3. peeking complete bytes without conversion returns a BytesChunk or SliceChunk
        if (b(iterator.getPosition()).get() % 8 == 0 && (length < b(0) || length.get() % 8 == 0)) {
            B startOffset = iterator.getPosition();
            B endOffset = iterator.getPosition() + (length < b(0) ? std::min(-length, chunkLength - iterator.getPosition()) : length);
            auto chunk = makeShared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + B(startOffset).get(), bytes.begin() + B(endOffset).get()));
            chunk->regionTags.copyTags(regionTags, iterator.getPosition(), b(0), chunk->getChunkLength());
            chunk->markImmutable();
            return chunk;
        }
        else
            // 4. peeking incomplete bytes without conversion returns a SliceChunk
            return peekConverted<SliceChunk>(iterator, length, flags);
    }
    // 4. peeking with conversion
    return converter(const_cast<BytesChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> BytesChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    b resultLength = length < b(0) ? std::min(-length, chunkLength - offset) : length;
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= resultLength && resultLength <= chunkLength);
    MemoryOutputStream outputStream(chunkLength);
    Chunk::serialize(outputStream, chunk, offset, resultLength);
    // TODO: this can return data which is contains garbage because the data length is not divisible by 8
    return makeShared<BytesChunk>(outputStream.getData());
}

void BytesChunk::setBytes(const std::vector<uint8_t>& bytes)
{
    handleChange();
    this->bytes = bytes;
}

void BytesChunk::setByte(int index, uint8_t byte)
{
    handleChange();
    bytes.at(index) = byte;
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

bool BytesChunk::canInsertAtFront(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTES;
}

bool BytesChunk::canInsertAtBack(const Ptr<const Chunk>& chunk) const
{
    return chunk->getChunkType() == CT_BYTES;
}

void BytesChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    const auto& bytesChunk = staticPtrCast<const BytesChunk>(chunk);
    bytes.insert(bytes.begin(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    const auto& bytesChunk = staticPtrCast<const BytesChunk>(chunk);
    bytes.insert(bytes.end(), bytesChunk->bytes.begin(), bytesChunk->bytes.end());
}

void BytesChunk::doRemoveAtFront(b length)
{
    bytes.erase(bytes.begin(), bytes.begin() + B(length).get());
}

void BytesChunk::doRemoveAtBack(b length)
{
    bytes.erase(bytes.end() - B(length).get(), bytes.end());
}

std::string BytesChunk::str() const
{
    std::ostringstream os;
    os << "BytesChunk, length = " << B(getChunkLength()) << ", bytes = {";
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
