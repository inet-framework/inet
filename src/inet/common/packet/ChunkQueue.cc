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

#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

ChunkQueue::ChunkQueue(const char *name, const std::shared_ptr<Chunk>& contents) :
    cNamedObject(name),
    contents(contents),
    iterator(Chunk::ForwardIterator(0, 0))
{
}

ChunkQueue::ChunkQueue(const ChunkQueue& other) :
    cNamedObject(other),
    contents(other.contents),
    iterator(other.iterator)
{
}

void ChunkQueue::remove(int64_t length)
{
    poppedByteCount += length;
    contents->moveIterator(iterator, length);
    auto position = iterator.getPosition();
    if (position > contents->getChunkLength() / 2) {
        contents->seekIterator(iterator, 0);
        contents = contents->peek(position, contents->getChunkLength() - position);
    }
}

std::shared_ptr<Chunk> ChunkQueue::peek(int64_t length) const
{
    assert(-1 <= length && length <= getBufferLength());
    return contents == nullptr ? nullptr : contents->peek(iterator, length);
}

std::shared_ptr<Chunk> ChunkQueue::peekAt(int64_t offset, int64_t length) const
{
    assert(0 <= offset && offset <= getBufferLength());
    assert(-1 <= length && length <= getBufferLength());
    return contents == nullptr ? nullptr : contents->peek(Chunk::Iterator(true, iterator.getPosition() + offset, -1), length);
}

std::shared_ptr<Chunk> ChunkQueue::pop(int64_t length)
{
    assert(-1 <= length && length <= getBufferLength());
    const auto& chunk = peek(length);
    if (chunk != nullptr)
        remove(chunk->getChunkLength());
    return chunk;
}

void ChunkQueue::clear()
{
    if (contents != nullptr) {
        poppedByteCount += getBufferLength();
        contents->seekIterator(iterator, 0);
        contents = nullptr;
    }
}

void ChunkQueue::push(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    pushedByteCount += chunk->getChunkLength();
    if (contents == nullptr)
        contents = chunk;
    else {
        if (contents->canInsertAtEnd(chunk)) {
            if (contents.use_count() == 1)
                contents->markMutableIfExclusivelyOwned();
            else
                contents = contents->dupShared();
            contents->insertAtEnd(chunk);
            contents = contents->peek(0, contents->getChunkLength());
        }
        else {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertAtEnd(contents);
            sequenceChunk->insertAtEnd(chunk);
            contents = sequenceChunk;
        }
        contents->markImmutable();
    }
}

} // namespace
