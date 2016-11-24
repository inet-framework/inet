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

void Buffer::remove(int64_t length)
{
    data->moveIterator(iterator, length);
    poppedLength += length;
    auto position = iterator.getPosition();
    if (position > data->getChunkLength() / 2) {
        data->removeFromBeginning(position);
        data->seekIterator(iterator, 0);
    }
}

std::shared_ptr<Chunk> Buffer::peek(int64_t length) const
{
    return data->peek(iterator, length);
}

std::shared_ptr<Chunk> Buffer::peekAt(int64_t offset, int64_t length) const
{
    return data->peek(Chunk::Iterator(true, offset), length);
}

std::shared_ptr<Chunk> Buffer::pop(int64_t length)
{
    const auto& chunk = peek(length);
    if (chunk != nullptr)
        remove(chunk->getChunkLength());
    return chunk;
}

void Buffer::push(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToEnd(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (!data->insertToEnd(chunk)) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToEnd(data);
            sequenceChunk->insertToEnd(chunk);
            data = sequenceChunk;
        }
    }
    pushedLength += chunk->getChunkLength();
}

} // namespace
