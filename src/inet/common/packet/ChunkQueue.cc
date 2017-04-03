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
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

ChunkQueue::ChunkQueue(const char *name, const std::shared_ptr<Chunk>& contents) :
    cNamedObject(name),
    contents(contents),
    iterator(Chunk::ForwardIterator(bit(0), 0))
{
}

ChunkQueue::ChunkQueue(const ChunkQueue& other) :
    cNamedObject(other),
    contents(other.contents),
    iterator(other.iterator)
{
}

void ChunkQueue::remove(bit length)
{
    assert(bit(0) <= length && length <= iterator.getPosition());
    if (contents->getChunkLength() == length)
        contents = EmptyChunk::singleton;
    else if (contents->canRemoveFromBeginning(length)) {
        contents = makeExclusivelyOwnedMutableChunk(contents);
        contents->removeFromBeginning(length);
        contents->markImmutable();
    }
    else
        contents = contents->peek(length, contents->getChunkLength() - length);
    contents->seekIterator(iterator, iterator.getPosition() - length);
    assert(isIteratorConsistent(iterator));
}

void ChunkQueue::moveIteratorOrRemove(bit length)
{
    poppedLength += length;
    contents->moveIterator(iterator, length);
    if (iterator.getPosition() > contents->getChunkLength() / 2)
        remove(iterator.getPosition());
}

std::shared_ptr<Chunk> ChunkQueue::peek(bit length, int flags) const
{
    assert(bit(-1) <= length && length <= getLength());
    return contents->peek(iterator, length, flags);
}

std::shared_ptr<Chunk> ChunkQueue::peekAt(bit offset, bit length, int flags) const
{
    assert(bit(0) <= offset && offset <= getLength());
    assert(bit(-1) <= length && offset + length <= getLength());
    return contents->peek(Chunk::Iterator(true, iterator.getPosition() + offset, -1), length, flags);
}

std::shared_ptr<Chunk> ChunkQueue::pop(bit length, int flags)
{
    assert(bit(-1) <= length && length <= getLength());
    const auto& chunk = peek(length, flags);
    if (chunk != nullptr)
        moveIteratorOrRemove(chunk->getChunkLength());
    return chunk;
}

void ChunkQueue::clear()
{
    poppedLength += getLength();
    contents->seekIterator(iterator, bit(0));
    contents = EmptyChunk::singleton;
}

void ChunkQueue::push(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    pushedLength += chunk->getChunkLength();
    if (contents == EmptyChunk::singleton)
        contents = chunk;
    else {
        if (contents->canInsertAtEnd(chunk)) {
            contents = makeExclusivelyOwnedMutableChunk(contents);
            contents->insertAtEnd(chunk);
            contents = contents->simplify();
        }
        else {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertAtEnd(contents);
            sequenceChunk->insertAtEnd(chunk);
            contents = sequenceChunk;
            contents->seekIterator(iterator, iterator.getPosition());
        }
        contents->markImmutable();
    }
    assert(isIteratorConsistent(iterator));
}

} // namespace
