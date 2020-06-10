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

namespace inet {

Register_Class(EmptyChunk);

const Ptr<EmptyChunk> EmptyChunk::singleton = makeShared<EmptyChunk>();

EmptyChunk::EmptyChunk() :
    Chunk()
{
    markImmutable();
}

EmptyChunk::EmptyChunk(const EmptyChunk& other) :
    Chunk(other)
{
    markImmutable();
}

const Ptr<Chunk> EmptyChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    CHUNK_CHECK_USAGE(iterator.getPosition() == b(0), "iterator is out of range");
    CHUNK_CHECK_USAGE(length <= b(0), "length is invalid");
    // 1. peeking returns nullptr
    if (predicate == nullptr || predicate(nullptr))
        return EmptyChunk::getEmptyChunk(flags);
    // 2. peeking returns this chunk
    auto result = const_cast<EmptyChunk *>(this)->shared_from_this();
    if (predicate == nullptr || predicate(result))
        return result;
    // 3. peeking with conversion
    return converter(const_cast<EmptyChunk *>(this)->shared_from_this(), iterator, length, flags);
}

const Ptr<Chunk> EmptyChunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    CHUNK_CHECK_IMPLEMENTATION(offset == b(0));
    CHUNK_CHECK_IMPLEMENTATION(length == b(0));
    return makeShared<EmptyChunk>();
}

std::string EmptyChunk::str() const
{
    return "EmptyChunk";
}

} // namespace
