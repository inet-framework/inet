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

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

SliceChunk::SliceChunk() :
    Chunk(),
    chunk(nullptr),
    offset(-1),
    length(-1)
{
}

SliceChunk::SliceChunk(const SliceChunk& other) :
    Chunk(other),
    chunk(other.chunk),
    offset(other.offset),
    length(other.length)
{
}

SliceChunk::SliceChunk(const Ptr<Chunk>& chunk, b offset, b length) :
    Chunk(),
    chunk(chunk),
    offset(offset),
    length(length < b(0) ? std::min(-length, chunk->getChunkLength() - offset) : length)
{
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
#if CHUNK_CHECK_IMPLEMENTATION_ENABLED
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= this->offset && this->offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= this->length && this->offset + this->length <= chunkLength);
#endif
    tags.copyTags(chunk->tags, offset, b(0), length);
}

void SliceChunk::forEachChild(cVisitor *v)
{
    Chunk::forEachChild(v);
    v->visit(const_cast<Chunk *>(chunk.get()));
}

const Ptr<Chunk> SliceChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    b chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(b(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == b(0) || (iterator.getPosition() == chunkLength && length < b(0))) {
        if (predicate == nullptr || predicate(nullptr))
            return EmptyChunk::getEmptyChunk(flags);
    }
    // 2. peeking the whole part
    if (iterator.getPosition() == b(0) && (-length >= chunkLength || length == chunkLength)) {
        // 2.1 peeking the whole part returns the sliced chunk
        if (offset == b(0) && chunkLength == chunk->getChunkLength()) {
            if (predicate == nullptr || predicate(chunk))
                return chunk;
        }
        // 2.2 peeking the whole part returns this chunk
        auto result = const_cast<SliceChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking anything else returns what peeking the sliced chunk returns
    return chunk->peekUnchecked(predicate, converter, ForwardIterator(iterator.getPosition() + offset, -1), length < b(0) ? -std::min(-length, chunkLength - offset) : length, flags);
}

const Ptr<Chunk> SliceChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    b chunkLength = chunk->getChunkLength();
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= offset && offset <= chunkLength);
    CHUNK_CHECK_IMPLEMENTATION(length <= chunkLength - offset);
    b sliceLength = length < b(0) ? std::min(-length, chunkLength) - offset : length;
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= sliceLength && sliceLength <= chunkLength);
    return makeShared<SliceChunk>(chunk, offset, sliceLength);
}

void SliceChunk::setOffset(b offset)
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= chunk->getChunkLength(), "offset is out of range");
    handleChange();
    this->offset = offset;
}

void SliceChunk::setLength(b length)
{
    CHUNK_CHECK_USAGE(b(0) <= length && length <= chunk->getChunkLength(), "length is invalid");
    handleChange();
    this->length = length;
}

bool SliceChunk::canInsertAtFront(const Ptr<const Chunk>& chunk) const
{
    if (chunk->getChunkType() == CT_SLICE) {
        const auto& otherSliceChunk = staticPtrCast<const SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length;
    }
    else
        return false;
}

bool SliceChunk::canInsertAtBack(const Ptr<const Chunk>& chunk) const
{
    if (chunk->getChunkType() == CT_SLICE) {
        const auto& otherSliceChunk = staticPtrCast<const SliceChunk>(chunk);
        return this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset;
    }
    else
        return false;
}

void SliceChunk::doInsertAtFront(const Ptr<const Chunk>& chunk)
{
    const auto& otherSliceChunk = staticPtrCast<const SliceChunk>(chunk);
    CHUNK_CHECK_IMPLEMENTATION(this->chunk == otherSliceChunk->chunk && offset == otherSliceChunk->offset + otherSliceChunk->length);
    offset -= otherSliceChunk->length;
    length += otherSliceChunk->length;
}

void SliceChunk::doInsertAtBack(const Ptr<const Chunk>& chunk)
{
    const auto& otherSliceChunk = staticPtrCast<const SliceChunk>(chunk);
    CHUNK_CHECK_IMPLEMENTATION(this->chunk == otherSliceChunk->chunk && offset + length == otherSliceChunk->offset);
    length += otherSliceChunk->length;
}

void SliceChunk::doRemoveAtFront(b length)
{
    this->offset += length;
    this->length -= length;
}

void SliceChunk::doRemoveAtBack(b length)
{
    this->length -= length;
}

std::string SliceChunk::str() const
{
    std::ostringstream os;
    os << "SliceChunk, offset = " << offset << ", length = " << length << ", chunk = {" << chunk << "}";
    return os.str();
}

} // namespace

