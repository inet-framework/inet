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

#include "Chunk.h"
#include "Serializer.h"

void Chunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    ByteOutputStream outputStream;
    chunk->serialize(outputStream);
    auto& outputBytes = outputStream.getBytes();
    auto begin = outputBytes.begin() + byteOffset;
    auto end = byteLength == -1 ? outputBytes.end() : outputBytes.begin() + byteOffset + byteLength;
    std::vector<uint8_t> inputBytes(begin, end);
    ByteInputStream inputStream(inputBytes);
    deserialize(inputStream);
}

void Chunk::serialize(ByteOutputStream& stream) const
{
    auto serializerClassName = getSerializerClassName();
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
    serializer->serialize(stream, *this);
    delete serializer;
}

void Chunk::deserialize(ByteInputStream& stream)
{
    auto serializerClassName = getSerializerClassName();
    auto serializer = dynamic_cast<ChunkSerializer *>(omnetpp::createOne(serializerClassName));
    serializer->deserialize(stream, *this);
    if (stream.isReadBeyondEnd())
        makeIncomplete();
    delete serializer;
}

void ByteArrayChunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    ByteOutputStream outputStream;
    chunk->serialize(outputStream);
    std::vector<uint8_t> chunkBytes;
    int byteCount = byteLength == -1 ? outputStream.getSize() : byteLength;
    for (int64_t i = 0; i < byteCount; i++)
        chunkBytes.push_back(outputStream[byteOffset + i]);
    setBytes(chunkBytes);
}

std::shared_ptr<Chunk> ByteArrayChunk::merge(const std::shared_ptr<Chunk>& other) const
{
    if (std::dynamic_pointer_cast<ByteArrayChunk>(other) != nullptr) {
        auto otherByteArrayChunk = std::static_pointer_cast<ByteArrayChunk>(other);
        std::vector<uint8_t> mergedBytes;
        mergedBytes.insert(mergedBytes.end(), bytes.begin(), bytes.end());
        mergedBytes.insert(mergedBytes.end(), otherByteArrayChunk->bytes.begin(), otherByteArrayChunk->bytes.end());
        return std::make_shared<ByteArrayChunk>(mergedBytes);
    }
    else
        return nullptr;
}

void ByteLengthChunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    setByteLength(byteLength);
}

std::shared_ptr<Chunk> ByteLengthChunk::merge(const std::shared_ptr<Chunk>& other) const
{
    if (std::dynamic_pointer_cast<ByteArrayChunk>(other) != nullptr) {
        auto otherByteLengthChunk = std::static_pointer_cast<ByteLengthChunk>(other);
        return std::make_shared<ByteLengthChunk>(byteLength + otherByteLengthChunk->byteLength);
    }
    else
        return nullptr;
}

void SliceChunk::replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength)
{
    this->chunk = chunk;
    this->byteOffset = byteOffset == -1 ? 0 : byteOffset;
    this->byteLength = byteLength == -1 ? chunk->getByteLength() - this->byteOffset : byteLength;
    this->chunk->assertImmutable();
    assert(this->byteOffset >= 0);
    assert(this->byteLength >= 0);
}

std::shared_ptr<Chunk> SliceChunk::merge(const std::shared_ptr<Chunk>& other) const
{
    if (std::dynamic_pointer_cast<SliceChunk>(other) != nullptr) {
        auto otherSliceChunk = std::static_pointer_cast<SliceChunk>(other);
        if (chunk == otherSliceChunk->getChunk() && byteOffset + byteLength == otherSliceChunk->getByteOffset()) {
            if (byteOffset == 0 && byteLength + otherSliceChunk->getByteLength() == chunk->getByteLength())
                return chunk;
            else
                return std::make_shared<SliceChunk>(chunk, byteOffset, byteLength + otherSliceChunk->getByteLength());
        }
        else
            return nullptr;
    }
    else
        return nullptr;
}

void SequenceChunk::makeImmutable()
{
    Chunk::makeImmutable();
    for (auto& chunk : chunks)
        chunk->makeImmutable();
}

int64_t SequenceChunk::getByteLength() const
{
    int64_t byteLength = 0;
    for (auto& chunk : chunks)
        byteLength += chunk->getByteLength();
    return byteLength;
}

void SequenceChunk::Iterator::seek(int64_t byteOffset)
{
    position = byteOffset;
    if (byteOffset == 0)
        index = 0;
    else {
        int startIndex = getStartIndex();
        int endIndex = getEndIndex();
        int increment = getIndexIncrement();
        int64_t p = 0;
        for (int i = startIndex; i != endIndex + increment; i += increment) {
            p += chunk->chunks[i]->getByteLength();
            if (p == byteOffset) {
                index = i + 1;
                return;
            }
        }
    }
}

void SequenceChunk::Iterator::move(int64_t byteLength)
{
    position += byteLength;
    if (index != -1 && index != chunk->chunks.size() && getChunk()->getByteLength() == byteLength)
        index++;
    else
        index = -1;
}

void SequenceChunk::doPrepend(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    chunks.insert(chunks.begin(), chunk);
}

void SequenceChunk::prepend(const std::shared_ptr<Chunk>& chunk)
{
    doPrepend(chunk);
}

void SequenceChunk::prepend(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto it = chunk->chunks.rbegin(); it != chunk->chunks.rend(); it++)
        doPrepend(*it);
}

void SequenceChunk::doAppend(const std::shared_ptr<Chunk>& chunk)
{
    assertMutable();
    const auto& mergedChunk = chunks.size() > 0 ? chunks.back()->merge(chunk) : nullptr;
    if (mergedChunk != nullptr)
        chunks.back() = mergedChunk;
    else
        chunks.push_back(chunk);
}

void SequenceChunk::append(const std::shared_ptr<Chunk>& chunk)
{
    doAppend(chunk);
}

void SequenceChunk::append(const std::shared_ptr<SliceChunk>& sliceChunk)
{
    int64_t byteOffset = 0;
    if (auto sequenceChunk = std::dynamic_pointer_cast<SequenceChunk>(sliceChunk->getChunk())) {
        for (auto& elementChunk : sequenceChunk->chunks) {
            int64_t sliceChunkBegin = sliceChunk->getByteOffset();
            int64_t sliceChunkEnd = sliceChunk->getByteOffset() + sliceChunk->getByteLength();
            int64_t chunkBegin = byteOffset;
            int64_t chunkEnd = byteOffset + elementChunk->getByteLength();
            if (sliceChunkBegin <= chunkBegin && chunkEnd <= sliceChunkEnd)
                doAppend(elementChunk);
            else if (chunkBegin < sliceChunkBegin && sliceChunkBegin < chunkEnd)
                doAppend(std::make_shared<SliceChunk>(elementChunk, sliceChunkBegin - chunkBegin, chunkEnd - sliceChunkBegin));
            else if (chunkBegin < sliceChunkEnd && sliceChunkEnd < chunkEnd)
                doAppend(std::make_shared<SliceChunk>(elementChunk, 0, chunkEnd - sliceChunkEnd));
            byteOffset += elementChunk->getByteLength();
        }
    }
    else
        doAppend(sliceChunk);
}

void SequenceChunk::append(const std::shared_ptr<SequenceChunk>& chunk)
{
    for (auto& chunk : chunk->chunks)
        doAppend(chunk);
}
