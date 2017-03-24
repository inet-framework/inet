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

// TODO: disable it by default
bool Chunk::enableImplicitChunkSerialization = true;
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
    assert(isMutable());
}

std::shared_ptr<Chunk> Chunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    if (!enableImplicitChunkSerialization)
        throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the value of the ENABLE_IMPLICIT_CHUNK_SERIALIZATION variable)");
    ByteOutputStream outputStream;
    serialize(outputStream, chunk, offset, length);
    ByteInputStream inputStream(outputStream.getBytes());
    return deserialize(inputStream, typeInfo);
}

std::shared_ptr<Chunk> Chunk::peek(const Iterator& iterator, bit length, int flags) const
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

void Chunk::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length)
{
    Chunk *chunkPointer = chunk.get();
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeid(*chunkPointer));
    auto startPosition = byte(stream.getPosition());
    serializer->serialize(stream, chunk, offset, length);
    auto endPosition = byte(stream.getPosition());
    auto expectedChunkLength = length == bit(-1) ? chunk->getChunkLength() - offset : length;
    assert(expectedChunkLength == endPosition - startPosition);
}

std::shared_ptr<Chunk> Chunk::deserialize(ByteInputStream& stream, const std::type_info& typeInfo)
{
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeInfo);
    auto startPosition = byte(stream.getPosition());
    auto chunk = serializer->deserialize(stream, typeInfo);
    auto endPosition = byte(stream.getPosition());
    assert(chunk->getChunkLength() == endPosition - startPosition);
    if (stream.isReadBeyondEnd())
        chunk->markIncomplete();
    return chunk;
}

} // namespace
