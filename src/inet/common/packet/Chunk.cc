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

namespace inet {

bool Chunk::enableImplicitChunkSerialization = true;

Chunk::Iterator::Iterator(int64_t position) :
    isForward_(true),
    position(position),
    index(position == 0 ? 0 : -1)
{
}

Chunk::Iterator::Iterator(bool isForward, int64_t position, int index) :
    isForward_(isForward),
    position(position),
    index(index)
{
}

Chunk::Iterator::Iterator(const Iterator& other) :
    isForward_(other.isForward_),
    position(other.position),
    index(other.index)
{
}

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
    ByteOutputStream outputStream;
    serialize(outputStream, chunk);
    auto& outputBytes = outputStream.getBytes();
    auto begin = outputBytes.begin() + offset;
    auto end = length == -1 ? outputBytes.end() : outputBytes.begin() + offset + length;
    std::vector<uint8_t> inputBytes(begin, end);
    ByteInputStream inputStream(inputBytes);
    return deserialize(inputStream, typeInfo);
}

void Chunk::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length)
{
    auto serializerClassName = chunk->getSerializerClassName();
    if (serializerClassName == nullptr)
        throw cRuntimeError("Serializer class is unspecified");
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
    if (serializer == nullptr)
        throw cRuntimeError("Serializer class not found");
    serializer->serialize(stream, chunk, offset, length);
    delete serializer;
}

std::shared_ptr<Chunk> Chunk::deserialize(ByteInputStream& stream, const std::type_info& typeInfo)
{
    auto serializerClassName = std::string(opp_demangle_typename(typeInfo.name())) + std::string("Serializer");
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName.c_str()));
    if (serializer == nullptr)
        throw cRuntimeError("Serializer class not found");
    auto chunk = serializer->deserialize(stream, typeInfo);
    delete serializer;
    if (stream.isReadBeyondEnd())
        chunk->makeIncomplete();
    return chunk;
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

} // namespace
