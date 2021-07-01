//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/packet/chunk/FieldsChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

FieldsChunk::FieldsChunk() :
    Chunk(),
    chunkLength(b(-1)),
    serializedBytes(nullptr)
{
}

FieldsChunk::FieldsChunk(const FieldsChunk& other) :
    Chunk(other),
    chunkLength(other.chunkLength),
    serializedBytes(other.serializedBytes != nullptr ? new std::vector<uint8_t>(*other.serializedBytes) : nullptr)
{
}

FieldsChunk::~FieldsChunk()
{
    delete serializedBytes;
}

void FieldsChunk::handleChange()
{
    Chunk::handleChange();
    delete serializedBytes;
    serializedBytes = nullptr;
}

const Ptr<Chunk> FieldsChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
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
        auto result = const_cast<FieldsChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking a part from the beginning without conversion returns an incomplete copy of this chunk
    if (predicate != nullptr && predicate(const_cast<FieldsChunk *>(this)->shared_from_this()) && iterator.getPosition() == b(0)) {
        auto copy = staticPtrCast<FieldsChunk>(dupShared());
        copy->setChunkLength(length);
        copy->tags.copyTags(tags, iterator.getPosition(), b(0), length);
        copy->markIncomplete();
        return copy;
    }
    // 4. peeking without conversion returns a SliceChunk
    if (converter == nullptr)
        return peekConverted<SliceChunk>(iterator, length, flags);
    // 5. peeking with conversion
    return converter(const_cast<FieldsChunk *>(this)->shared_from_this(), iterator, length, flags);
}

} // namespace
