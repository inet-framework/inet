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

#include "inet/common/packet/FifoChunkBuffer.h"
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

FifoChunkBuffer::FifoChunkBuffer(const char *name, const std::shared_ptr<Chunk>& contents) :
    cNamedObject(name),
    contents(contents),
    iterator(Chunk::ForwardIterator(0, 0))
{
}

FifoChunkBuffer::FifoChunkBuffer(const FifoChunkBuffer& other) :
    cNamedObject(other),
    contents(other.isImmutable() ? other.contents : other.contents->dupShared()),
    iterator(other.iterator)
{
}

void FifoChunkBuffer::remove(int64_t length)
{
    contents->moveIterator(iterator, length);
    poppedByteCount += length;
    auto position = iterator.getPosition();
    if (position > contents->getChunkLength() / 2) {
        contents->removeFromBeginning(position);
        contents->seekIterator(iterator, 0);
    }
}

std::shared_ptr<Chunk> FifoChunkBuffer::peek(int64_t length) const
{
    return contents->peek(iterator, length);
}

std::shared_ptr<Chunk> FifoChunkBuffer::peekAt(int64_t offset, int64_t length) const
{
    return contents->peek(Chunk::Iterator(true, iterator.getPosition() + offset, -1), length);
}

std::shared_ptr<Chunk> FifoChunkBuffer::pop(int64_t length)
{
    const auto& chunk = peek(length);
    if (chunk != nullptr)
        remove(chunk->getChunkLength());
    return chunk;
}

void FifoChunkBuffer::push(const std::shared_ptr<Chunk>& chunk)
{
    if (contents == nullptr)
        contents = chunk->isImmutable() ? chunk->dupShared() : chunk;
    else {
        if (!contents->insertAtEnd(chunk)) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertAtEnd(contents);
            sequenceChunk->insertAtEnd(chunk);
            contents = sequenceChunk;
        }
    }
    pushedByteCount += chunk->getChunkLength();
}

} // namespace
