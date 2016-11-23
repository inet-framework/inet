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

#include "inet/common/packet/Buffer.h"
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

Buffer::Buffer() :
    data(nullptr)
{
}

Buffer::Buffer(const Buffer& other) :
    data(other.isImmutable() ? other.data : other.data->dupShared()),
    iterator(other.iterator)
{
}

Buffer::Buffer(const std::shared_ptr<Chunk>& data) :
    data(data)
{
}

void Buffer::remove(int64_t byteLength)
{
    data->moveIterator(iterator, byteLength);
    poppedByteLength += byteLength;
    auto position = iterator.getPosition();
    if (position > data->getChunkLength() / 2) {
        data->removeFromBeginning(position);
        data->seekIterator(iterator, 0);
    }
}

std::shared_ptr<Chunk> Buffer::peek(int64_t byteLength) const
{
    return data->peek(iterator, byteLength);
}

std::shared_ptr<Chunk> Buffer::peekAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(Chunk::Iterator(true, byteOffset), byteLength);
}

std::shared_ptr<Chunk> Buffer::pop(int64_t byteLength)
{
    const auto& chunk = peek(byteLength);
    if (chunk != nullptr)
        remove(chunk->getChunkLength());
    return chunk;
}

void Buffer::push(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->append(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (data->getChunkType() == Chunk::TYPE_SEQUENCE)
            std::static_pointer_cast<SequenceChunk>(data)->append(chunk);
        else {
            if (!data->insertToEnd(chunk)) {
                auto sequenceChunk = std::make_shared<SequenceChunk>();
                sequenceChunk->append(data);
                sequenceChunk->append(chunk);
                data = sequenceChunk;
            }
        }
    }
    pushedByteLength += chunk->getChunkLength();
}

} // namespace
