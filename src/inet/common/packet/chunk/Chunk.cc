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

#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/serializer/ChunkSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

bool Chunk::enableImplicitChunkSerialization = false;
int Chunk::nextId = 0;

Chunk::Chunk() :
    id(nextId++),
    flags(0)
{
}

Chunk::Chunk(const Chunk& other) :
    id(nextId++),
    flags(other.flags & ~CF_IMMUTABLE)
{
}

void Chunk::handleChange()
{
    CHUNK_CHECK_USAGE(isMutable(), "chunk is immutable");
}

Ptr<Chunk> Chunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, bit offset, bit length, int flags)
{
    auto chunkType = chunk->getChunkType();
    if (!enableImplicitChunkSerialization && !(flags & PF_ALLOW_SERIALIZATION) && chunkType != CT_BITS && chunkType != CT_BYTES)
        throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the Chunk::enableImplicitChunkSerialization flag or passing the PF_ALLOW_SERIALIZATION flat to peek)");
    MemoryOutputStream outputStream;
    serialize(outputStream, chunk, offset, length);
    MemoryInputStream inputStream(outputStream.getData());
    return deserialize(inputStream, typeInfo);
}

void Chunk::moveIterator(Iterator& iterator, bit length) const
{
    auto position = iterator.getPosition() + length;
    iterator.setPosition(position);
    iterator.setIndex(position == bit(0) ? 0 : -1);
}

void Chunk::seekIterator(Iterator& iterator, bit position) const
{
    iterator.setPosition(position);
    iterator.setIndex(position == bit(0) ? 0 : -1);
}

Ptr<Chunk> Chunk::peek(const Iterator& iterator, bit length, int flags) const
{
    const auto& chunk = peekUnchecked(nullptr, nullptr, iterator, length, flags);
    return checkPeekResult<Chunk>(chunk, flags);
}

std::string Chunk::str() const
{
    std::ostringstream os;
    os << getClassName() << ", length = " << getChunkLength();
    return os.str();
}

void Chunk::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk, bit offset, bit length)
{
    Chunk *chunkPointer = chunk.get();
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeid(*chunkPointer));
    auto startPosition = stream.getLength();
    serializer->serialize(stream, chunk, offset, length);
    auto endPosition = stream.getLength();
    auto expectedChunkLength = length == bit(-1) ? chunk->getChunkLength() - offset : length;
    CHUNK_CHECK_IMPLEMENTATION(expectedChunkLength == endPosition - startPosition);
}

Ptr<Chunk> Chunk::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo)
{
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeInfo);
    auto startPosition = byte(stream.getPosition());
    auto chunk = serializer->deserialize(stream, typeInfo);
    auto endPosition = byte(stream.getPosition());
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkLength() == endPosition - startPosition);
    if (stream.isReadBeyondEnd())
        chunk->markIncomplete();
    return chunk;
}

} // namespace
