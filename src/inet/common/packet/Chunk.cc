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
    flags(0),
    serializedBytes(nullptr)
{
}

Chunk::Chunk(const Chunk& other) :
    flags(other.flags & ~FLAG_IMMUTABLE),
    serializedBytes(other.serializedBytes != nullptr ? new std::vector<uint8_t>(*other.serializedBytes) : nullptr)
{
}

Chunk::~Chunk()
{
    delete serializedBytes;
}

void Chunk::handleChange()
{
    assertMutable();
    delete serializedBytes;
    serializedBytes = nullptr;
}

std::shared_ptr<Chunk> Chunk::createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    ByteOutputStream outputStream;
    serialize(outputStream, chunk);
    auto& outputBytes = outputStream.getBytes();
    auto begin = outputBytes.begin() + byteOffset;
    auto end = byteLength == -1 ? outputBytes.end() : outputBytes.begin() + byteOffset + byteLength;
    std::vector<uint8_t> inputBytes(begin, end);
    ByteInputStream inputStream(inputBytes);
    return deserialize(inputStream, typeInfo);
}

void Chunk::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk)
{
    if (chunk->serializedBytes != nullptr)
        stream.writeBytes(*chunk->serializedBytes);
    else {
        auto streamPosition = stream.getPosition();
        auto serializerClassName = chunk->getSerializerClassName();
        auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
        serializer->serialize(stream, chunk);
        delete serializer;
        auto byteLength = stream.getPosition() - streamPosition;
        chunk->serializedBytes = stream.copyBytes(streamPosition, byteLength);
        ChunkSerializer::totalSerializedByteLength += byteLength;
    }
}

std::shared_ptr<Chunk> Chunk::deserialize(ByteInputStream& stream, const std::type_info& typeInfo)
{
    auto streamPosition = stream.getPosition();
    auto serializerClassName = std::string(opp_demangle_typename(typeInfo.name())) + std::string("Serializer");
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName.c_str()));
    auto chunk = serializer->deserialize(stream);
    delete serializer;
    if (stream.isReadBeyondEnd())
        chunk->makeIncomplete();
    auto byteLength = stream.getPosition() - streamPosition;
    chunk->serializedBytes = stream.copyBytes(streamPosition, byteLength);
    ChunkSerializer::totalDeserializedByteLength += byteLength;
    return chunk;
}

std::shared_ptr<Chunk> Chunk::peek(const Iterator& iterator, int64_t byteLength) const
{
    if (iterator.getPosition() == 0 && byteLength == getByteLength())
        return const_cast<Chunk *>(this)->shared_from_this();
    else
        return peek<SliceChunk>(iterator, byteLength);
}

std::string Chunk::str() const
{
    std::ostringstream os;
    os << "Chunk, byteLength = " << getByteLength();
    return os.str();
}

} // namespace
