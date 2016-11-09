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

namespace inet {

Buffer::Buffer() :
    data(std::make_shared<SequenceChunk>()),
    iterator(data->createForwardIterator())
{
}

Buffer::Buffer(const Buffer& other) :
    data(other.isImmutable() ? other.data : std::make_shared<SequenceChunk>(*other.data)),
    iterator(other.iterator)
{
}

void Buffer::remove(int64_t byteLength)
{
    iterator.move(byteLength);
    poppedByteLength += byteLength;
    auto position = iterator.getPosition();
    if (position > data->getByteLength() / 2) {
        data->removeBeginning(position);
        iterator.seek(0);
    }
}

std::shared_ptr<Chunk> Buffer::peek(int64_t byteLength) const
{
    return data->peek(iterator, byteLength);
}

std::shared_ptr<Chunk> Buffer::peekAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(SequenceChunk::ForwardIterator(data, -1, byteOffset), byteLength);
}

void Buffer::push(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    data->append(chunk, flatten);
    pushedByteLength += chunk->getByteLength();
}

void Buffer::push(Buffer* buffer, bool flatten)
{
    data->append(buffer->data, flatten);
    pushedByteLength += buffer->getByteLength();
}

} // namespace
