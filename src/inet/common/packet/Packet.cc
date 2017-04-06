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

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Register_Class(Packet);

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    contents(EmptyChunk::singleton),
    headerIterator(Chunk::ForwardIterator(bit(0), 0)),
    trailerIterator(Chunk::BackwardIterator(bit(0), 0))
{
}

Packet::Packet(const char *name, const std::shared_ptr<Chunk>& contents) :
    cPacket(name),
    contents(contents),
    headerIterator(Chunk::ForwardIterator(bit(0), 0)),
    trailerIterator(Chunk::BackwardIterator(bit(0), 0))
{
    CHUNK_CHECK_IMPLEMENTATION(contents->isImmutable());
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    contents(other.contents),
    headerIterator(other.headerIterator),
    trailerIterator(other.trailerIterator)
{
}

void Packet::setHeaderPopOffset(bit offset)
{
    CHUNK_CHECK_USAGE(bit(0) <= offset && offset <= getTotalLength() - trailerIterator.getPosition(), "offset is out of range");
    contents->seekIterator(headerIterator, offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= bit(0));
}

std::shared_ptr<Chunk> Packet::peekHeader(bit length, int flags) const
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    return contents->peek(headerIterator, length, flags);
}

std::shared_ptr<Chunk> Packet::popHeader(bit length, int flags)
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekHeader(length, flags);
    if (chunk != nullptr)
        contents->moveIterator(headerIterator, chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::removeHeader(bit length, int flags)
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == bit(0), "popped header length is non-zero");
    const auto& chunk = popHeader(length, flags);
    removePoppedHeaders();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

void Packet::pushHeader(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    prepend(chunk);
}

void Packet::insertHeader(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    chunk->markImmutable();
    prepend(chunk);
}

void Packet::setTrailerPopOffset(bit offset)
{
    CHUNK_CHECK_USAGE(headerIterator.getPosition() <= offset, "offset is out of range");
    contents->seekIterator(trailerIterator, getTotalLength() - offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= bit(0));
}

std::shared_ptr<Chunk> Packet::peekTrailer(bit length, int flags) const
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    return contents->peek(trailerIterator, length, flags);
}

std::shared_ptr<Chunk> Packet::popTrailer(bit length, int flags)
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekTrailer(length, flags);
    if (chunk != nullptr)
        contents->moveIterator(trailerIterator, -chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::removeTrailer(bit length, int flags)
{
    CHUNK_CHECK_USAGE(bit(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == bit(0), "popped trailer length is non-zero");
    const auto& chunk = popTrailer(length, flags);
    removePoppedTrailers();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

void Packet::pushTrailer(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    append(chunk);
}

void Packet::insertTrailer(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    chunk->markImmutable();
    append(chunk);
}

std::shared_ptr<Chunk> Packet::peekDataAt(bit offset, bit length, int flags) const
{
    CHUNK_CHECK_USAGE(bit(0) <= offset && offset <= getDataLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(bit(-1) <= length && offset + length <= getDataLength(), "length is invalid");
    bit peekOffset = headerIterator.getPosition() + offset;
    bit peekLength = length == bit(-1) ? getDataLength() - offset : length;
    return contents->peek(Chunk::Iterator(true, peekOffset, -1), peekLength, flags);
}

std::shared_ptr<Chunk> Packet::peekAt(bit offset, bit length, int flags) const
{
    CHUNK_CHECK_USAGE(bit(0) <= offset && offset <= getTotalLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(bit(-1) <= length && offset + length <= getTotalLength(), "length is invalid");
    bit peekLength = length == bit(-1) ? getTotalLength() - offset : length;
    return contents->peek(Chunk::Iterator(true, offset, -1), peekLength, flags);
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == bit(0) && (headerIterator.getIndex() == 0 || headerIterator.getIndex() == -1), "popped header length is non-zero");
    if (contents == EmptyChunk::singleton)
        contents = chunk;
    else {
        if (contents->canInsertAtBeginning(chunk)) {
            contents = makeExclusivelyOwnedMutableChunk(contents);
            contents->insertAtBeginning(chunk);
            contents = contents->simplify();
        }
        else {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertAtBeginning(contents);
            sequenceChunk->insertAtBeginning(chunk);
            contents = sequenceChunk;
        }
        contents->markImmutable();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::append(const std::shared_ptr<Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == bit(0) && (trailerIterator.getIndex() == 0 || trailerIterator.getIndex() == -1), "popped trailer length is non-zero");
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
        }
        contents->markImmutable();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removeFromBeginning(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getTotalLength() - trailerIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == bit(0) && (headerIterator.getIndex() == 0 || headerIterator.getIndex() == -1), "popped header length is non-zero");
    if (contents->getChunkLength() == length)
        contents = EmptyChunk::singleton;
    else if (contents->canRemoveFromBeginning(length)) {
        contents = makeExclusivelyOwnedMutableChunk(contents);
        contents->removeFromBeginning(length);
        contents->markImmutable();
    }
    else
        contents = contents->peek(length, contents->getChunkLength() - length);
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removeFromEnd(bit length)
{
    CHUNK_CHECK_USAGE(bit(0) <= length && length <= getTotalLength() - headerIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == bit(0) && (trailerIterator.getIndex() == 0 || trailerIterator.getIndex() == -1), "popped trailer length is non-zero");
    if (contents->getChunkLength() == length)
        contents = EmptyChunk::singleton;
    else if (contents->canRemoveFromEnd(length)) {
        contents = makeExclusivelyOwnedMutableChunk(contents);
        contents->removeFromEnd(length);
        contents->markImmutable();
    }
    else
        contents = contents->peek(bit(0), contents->getChunkLength() - length);
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removePoppedHeaders()
{
    bit poppedLength = getHeaderPoppedLength();
    setHeaderPopOffset(bit(0));
    removeFromBeginning(poppedLength);
}

void Packet::removePoppedTrailers()
{
    bit poppedLength = getTrailerPoppedLength();
    setTrailerPopOffset(getTotalLength());
    removeFromEnd(poppedLength);
}

void Packet::removePoppedChunks()
{
    removePoppedHeaders();
    removePoppedTrailers();
}

} // namespace
