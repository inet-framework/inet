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
const b Chunk::unspecifiedLength = b(-std::numeric_limits<int64_t>::max());

Chunk::Chunk() :
    id(nextId++),
    flags(0)
{
}

Chunk::Chunk(const Chunk& other) :
    id(nextId++),
    flags(other.flags & ~CF_IMMUTABLE),
    tags(other.tags)
{
}

void Chunk::forEachChild(cVisitor *v)
{
    for (int i = 0; i < tags.getNumTags(); i++)
        v->visit(tags.getTag(i));
}

void Chunk::handleChange()
{
    checkMutable();
}

int Chunk::getBinDumpNumLines()
{
    return (b(getChunkLength()).get() + 31) / 32;
}

int Chunk::getHexDumpNumLines()
{
    return ((b(getChunkLength()).get() + 7) / 8 + 15) / 16;
}

static std::string asStringValue;

const char *Chunk::getBinDumpLine(int index)
{
    asStringValue = "";
    try {
        int offset = index * 32;
        int length = std::min(32, (int)b(getChunkLength()).get() - offset);
        MemoryOutputStream outputStream;
        serialize(outputStream, shared_from_this(), b(offset), b(length));
        std::vector<bool> bits;
        outputStream.copyData(bits);
        for (int i = 0; i < length; i++) {
            if (i != 0 && i % 4 == 0)
                asStringValue += " ";
            asStringValue += (bits[i] ? "1" : "0");
        }
    }
    catch (cRuntimeError& e) {
        asStringValue = e.what();
    }
    return asStringValue.c_str();
}

const char *Chunk::getHexDumpLine(int index)
{
    asStringValue = "";
    try {
        int offset = index * 8 * 16;
        int length = std::min(8 * 16, (int)b(getChunkLength()).get() - offset);
        MemoryOutputStream outputStream;
        serialize(outputStream, shared_from_this(), b(offset), b(length));
        ASSERT(outputStream.getLength() == b(length));
        std::vector<uint8_t> bytes;
        outputStream.copyData(bytes);
        char tmp[3] = "  ";
        for (size_t i = 0; i < bytes.size(); i++) {
            if (i != 0)
                asStringValue += " ";
            sprintf(tmp, "%02X", bytes[i]);
            asStringValue += tmp;
        }
    }
    catch (cRuntimeError& e) {
        asStringValue = e.what();
    }
    return asStringValue.c_str();
}

int Chunk::getTagsArraySize()
{
    return tags.getNumTags();
}

const RegionTagSet::RegionTag<cObject>& Chunk::getTags(int index)
{
    return tags.getRegionTag(index);
}

const Ptr<Chunk> Chunk::convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags)
{
    auto chunkType = chunk->getChunkType();
    if (!enableImplicitChunkSerialization && !(flags & PF_ALLOW_SERIALIZATION) && chunkType != CT_BITS && chunkType != CT_BYTES)
        throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the Chunk::enableImplicitChunkSerialization flag or passing the PF_ALLOW_SERIALIZATION flag to peek)");
    MemoryOutputStream outputStream;
    serialize(outputStream, chunk, offset, length < b(0) ? std::min(-length, chunk->getChunkLength() - offset) : length);
    MemoryInputStream inputStream(outputStream.getData());
    const auto& result = deserialize(inputStream, typeInfo);
    result->tags.copyTags(chunk->tags, offset, b(0), result->getChunkLength());
    return result;
}

void Chunk::moveIterator(Iterator& iterator, b length) const
{
    auto position = iterator.getPosition() + length;
    iterator.setPosition(position);
    iterator.setIndex(position == b(0) ? 0 : -1);
}

void Chunk::seekIterator(Iterator& iterator, b position) const
{
    iterator.setPosition(position);
    iterator.setIndex(position == b(0) ? 0 : -1);
}

const Ptr<Chunk> Chunk::peek(const Iterator& iterator, b length, int flags) const
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

void Chunk::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length)
{
    CHUNK_CHECK_USAGE(length >= b(-1), "length is invalid");
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= chunk->getChunkLength(), "offset is out of range");
    const Chunk *chunkPointer = chunk.get();
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeid(*chunkPointer));
#if CHUNK_CHECK_IMPLEMENTATION_ENABLED
    auto startPosition = stream.getLength();
#endif
    serializer->serialize(stream, chunk, offset, length);
#if CHUNK_CHECK_IMPLEMENTATION_ENABLED
    auto endPosition = stream.getLength();
    auto expectedChunkLength = length == b(-1) ? chunk->getChunkLength() - offset : length;
    CHUNK_CHECK_IMPLEMENTATION(expectedChunkLength == endPosition - startPosition);
#endif
}

const Ptr<Chunk> Chunk::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo)
{
    auto serializer = ChunkSerializerRegistry::globalRegistry.getSerializer(typeInfo);
#if CHUNK_CHECK_IMPLEMENTATION_ENABLED
    auto startPosition = B(stream.getPosition());
#endif
    auto chunk = serializer->deserialize(stream, typeInfo);
#if CHUNK_CHECK_IMPLEMENTATION_ENABLED
    auto endPosition = B(stream.getPosition());
    CHUNK_CHECK_IMPLEMENTATION(chunk->getChunkLength() == endPosition - startPosition);
#endif
    if (stream.isReadBeyondEnd())
        chunk->markIncomplete();
    return chunk;
}

} // namespace
