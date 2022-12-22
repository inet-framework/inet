//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/ChunkQueue.h"

#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

ChunkQueue::ChunkQueue(const char *name, const Ptr<const Chunk>& content) :
    cNamedObject(name),
    content(content),
    iterator(Chunk::ForwardIterator(b(0), 0))
{
    constPtrCast<Chunk>(content)->markImmutable();
}

ChunkQueue::ChunkQueue(const ChunkQueue& other) :
    cNamedObject(other),
    content(other.content),
    iterator(other.iterator)
{
    CHUNK_CHECK_IMPLEMENTATION(content->isImmutable());
}

void ChunkQueue::remove(b length)
{
    CHUNK_CHECK_IMPLEMENTATION(b(0) <= length && length <= iterator.getPosition());
    if (content->getChunkLength() == length)
        content = makeShared<EmptyChunk>();
    else if (content->canRemoveAtFront(length)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAtFront(length);
        newContent->markImmutable();
        content = newContent;
    }
    else
        content = content->peek(length, content->getChunkLength() - length);
    content->seekIterator(iterator, iterator.getPosition() - length);
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(iterator));
}

void ChunkQueue::moveIteratorOrRemove(b length)
{
    poppedLength += length;
    content->moveIterator(iterator, length);
    if (iterator.getPosition() > content->getChunkLength() / 2)
        remove(iterator.getPosition());
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(iterator));
}

const Ptr<const Chunk> ChunkQueue::peek(b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getLength(), "length is invalid");
    return content->peek(iterator, length == b(-1) ? Chunk::unspecifiedLength : length, flags);
}

const Ptr<const Chunk> ChunkQueue::peekAt(b offset, b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getLength(), "length is invalid");
    return content->peek(Chunk::Iterator(true, iterator.getPosition() + offset, -1), length == b(-1) ? Chunk::unspecifiedLength : length, flags);
}

const Ptr<const Chunk> ChunkQueue::pop(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getLength(), "length is invalid");
    const auto& chunk = peek(length == b(-1) ? Chunk::unspecifiedLength : length, flags);
    if (chunk != nullptr)
        moveIteratorOrRemove(chunk->getChunkLength());
    return chunk;
}

void ChunkQueue::clear()
{
    poppedLength += getLength();
    content->seekIterator(iterator, b(0));
    content = makeShared<EmptyChunk>();
}

void ChunkQueue::push(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
    constPtrCast<Chunk>(chunk)->markImmutable();
    pushedLength += chunk->getChunkLength();
    if (content->getChunkType() == Chunk::CT_EMPTY)
        content = chunk;
    else {
        if (content->canInsertAtBack(chunk)) {
            const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
            newContent->insertAtBack(chunk);
            newContent->markImmutable();
            content = newContent->simplify();
        }
        else {
            auto sequenceChunk = makeShared<SequenceChunk>();
            sequenceChunk->insertAtBack(content);
            sequenceChunk->insertAtBack(chunk);
            sequenceChunk->markImmutable();
            content = sequenceChunk;
            content->seekIterator(iterator, iterator.getPosition());
        }
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(iterator));
}

std::string ChunkQueue::str() const
{
    std::stringstream stream;
    if (iterator.getPosition() == b(0))
        stream << content;
    else
        stream << content->peek(iterator);
    return stream.str();
}

} // namespace

