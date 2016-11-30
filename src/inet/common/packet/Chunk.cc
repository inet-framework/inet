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

#include "inet/common/packet/Chunk.h"
#include "inet/common/packet/Serializer.h"
#include "inet/common/packet/SerializerRegistry.h"

namespace inet {

// TODO: disable it by default
bool Chunk::enableImplicitChunkSerialization = true;

Chunk::Chunk() :
    flags(0)
{
}

Chunk::Chunk(const Chunk& other) :
    flags(other.flags & ~FLAG_IMMUTABLE)
{
}

void Chunk::handleChange()
{
    assertMutable();
}

std::shared_ptr<Chunk> Chunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    if (!enableImplicitChunkSerialization)
        throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the value of the ENABLE_IMPLICIT_CHUNK_SERIALIZATION variable)");
    ByteOutputStream outputStream;
    serialize(outputStream, chunk, offset, length);
    ByteInputStream inputStream(outputStream.getBytes());
    return deserialize(inputStream, typeInfo);
}

std::shared_ptr<Chunk> Chunk::peek(const Iterator& iterator, int64_t length) const
{
    if (iterator.getPosition() == 0 && (length == -1 || length == getChunkLength()))
        return const_cast<Chunk *>(this)->shared_from_this();
    else
        return peek<SliceChunk>(iterator, length);
}

std::string Chunk::str() const
{
    std::ostringstream os;
    os << "Chunk, length = " << getChunkLength();
    return os.str();
}

void Chunk::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    Chunk *chunkPointer = chunk.get();
    auto serializer = SerializerRegistry::globalRegistry.getSerializer(typeid(*chunkPointer));
    serializer->serialize(stream, chunk, offset, length);
}

std::shared_ptr<Chunk> Chunk::deserialize(ByteInputStream& stream, const std::type_info& typeInfo)
{
    auto serializer = SerializerRegistry::globalRegistry.getSerializer(typeInfo);
    auto chunk = serializer->deserialize(stream, typeInfo);
    if (stream.isReadBeyondEnd())
        chunk->makeIncomplete();
    return chunk;
}

} // namespace
