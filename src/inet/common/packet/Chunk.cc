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

Chunk::Chunk(const Chunk& other) :
    isImmutable_(other.isImmutable_),
    isIncomplete_(other.isIncomplete_)
{
}

std::shared_ptr<Chunk> Chunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    // TODO: prevent unnecessary complete serialization, and serialize only relevant parts
    ByteOutputStream outputStream;
    chunk->serialize(outputStream);
    auto& outputBytes = outputStream.getBytes();
    auto begin = outputBytes.begin() + byteOffset;
    auto end = byteLength == -1 ? outputBytes.end() : outputBytes.begin() + byteOffset + byteLength;
    std::vector<uint8_t> inputBytes(begin, end);
    ByteInputStream inputStream(inputBytes);
    return deserialize(inputStream);
}

void Chunk::serialize(ByteOutputStream& stream) const
{
    auto serializerClassName = getSerializerClassName();
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
    serializer->serialize(stream, *this);
    delete serializer;
}

std::shared_ptr<Chunk> Chunk::deserialize(ByteInputStream& stream)
{
    auto serializerClassName = getSerializerClassName();
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
    auto chunk = serializer->deserialize(stream);
    delete serializer;
    if (stream.isReadBeyondEnd())
        chunk->makeIncomplete();
    return chunk;
}

std::string Chunk::str() const
{
    std::ostringstream os;
    os << "Chunk, byteLength = " << getByteLength();
    return os.str();
}

} // namespace
