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
    headerIterator(Chunk::ForwardIterator(b(0), 0)),
    trailerIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(b(0))
{
    CHUNK_CHECK_IMPLEMENTATION(contents->isImmutable());
}

Packet::Packet(const char *name, const Ptr<const Chunk>& contents) :
    cPacket(name),
    contents(contents),
    headerIterator(Chunk::ForwardIterator(b(0), 0)),
    trailerIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(contents->getChunkLength())
{
    CHUNK_CHECK_IMPLEMENTATION(contents->isImmutable());
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    contents(other.contents),
    headerIterator(other.headerIterator),
    trailerIterator(other.trailerIterator),
    totalLength(other.totalLength)
{
    CHUNK_CHECK_IMPLEMENTATION(contents->isImmutable());
}

void Packet::forEachChild(cVisitor *v)
{
    v->visit(const_cast<Chunk *>(contents.get()));
}

void Packet::setHeaderPopOffset(b offset)
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength() - trailerIterator.getPosition(), "offset is out of range");
    contents->seekIterator(headerIterator, offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
}

const Ptr<const Chunk> Packet::peekHeader(b length, int flags) const
{
    auto dataLength = getDataLength();
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
    const auto& chunk = contents->peek(headerIterator, length, flags);
    if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
        return chunk;
    else
        return contents->peek(headerIterator, dataLength, flags);
}

const Ptr<const Chunk> Packet::popHeader(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekHeader(length, flags);
    if (chunk != nullptr) {
        contents->moveIterator(headerIterator, chunk->getChunkLength());
        CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
    }
    return chunk;
}

const Ptr<Chunk> Packet::removeHeader(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == b(0), "popped header length is non-zero");
    const auto& chunk = popHeader(length, flags);
    removePoppedHeaders();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

void Packet::pushHeader(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    prepend(chunk);
}

void Packet::insertHeader(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    constPtrCast<Chunk>(chunk)->markImmutable();
    prepend(chunk);
}

void Packet::setTrailerPopOffset(b offset)
{
    CHUNK_CHECK_USAGE(headerIterator.getPosition() <= offset, "offset is out of range");
    contents->seekIterator(trailerIterator, getTotalLength() - offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
}

const Ptr<const Chunk> Packet::peekTrailer(b length, int flags) const
{
    auto dataLength = getDataLength();
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
    const auto& chunk = contents->peek(trailerIterator, length, flags);
    if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
        return chunk;
    else
        return contents->peek(trailerIterator, dataLength, flags);
}

const Ptr<const Chunk> Packet::popTrailer(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekTrailer(length, flags);
    if (chunk != nullptr) {
        contents->moveIterator(trailerIterator, chunk->getChunkLength());
        CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
    }
    return chunk;
}

const Ptr<Chunk> Packet::removeTrailer(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == b(0), "popped trailer length is non-zero");
    const auto& chunk = popTrailer(length, flags);
    removePoppedTrailers();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

void Packet::pushTrailer(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    append(chunk);
}

void Packet::insertTrailer(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    constPtrCast<Chunk>(chunk)->markImmutable();
    append(chunk);
}

const Ptr<const Chunk> Packet::peekDataAt(b offset, b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
    b peekOffset = headerIterator.getPosition() + offset;
    b peekLength = length == b(-1) ? getDataLength() - offset : length;
    return contents->peek(Chunk::Iterator(true, peekOffset, -1), peekLength, flags);
}

const Ptr<const Chunk> Packet::peekAt(b offset, b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getTotalLength(), "length is invalid");
    b peekLength = length == b(-1) ? getTotalLength() - offset : length;
    return contents->peek(Chunk::Iterator(true, offset, -1), peekLength, flags);
}

void Packet::prepend(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == b(0) && (headerIterator.getIndex() == 0 || headerIterator.getIndex() == -1), "popped header length is non-zero");
    if (contents == EmptyChunk::singleton) {
        contents = chunk;
        totalLength = contents->getChunkLength();
    }
    else {
        if (contents->canInsertAtBeginning(chunk)) {
            const auto& newContents = makeExclusivelyOwnedMutableChunk(contents);
            newContents->insertAtBeginning(chunk);
            newContents->markImmutable();
            contents = newContents->simplify();
        }
        else {
            auto sequenceChunk = makeShared<SequenceChunk>();
            sequenceChunk->insertAtBeginning(contents);
            sequenceChunk->insertAtBeginning(chunk);
            sequenceChunk->markImmutable();
            contents = sequenceChunk;
        }
        totalLength += chunk->getChunkLength();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::append(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == b(0) && (trailerIterator.getIndex() == 0 || trailerIterator.getIndex() == -1), "popped trailer length is non-zero");
    if (contents == EmptyChunk::singleton) {
        contents = chunk;
        totalLength = contents->getChunkLength();
    }
    else {
        if (contents->canInsertAtEnd(chunk)) {
            const auto& newContents = makeExclusivelyOwnedMutableChunk(contents);
            newContents->insertAtEnd(chunk);
            newContents->markImmutable();
            contents = newContents->simplify();
        }
        else {
            auto sequenceChunk = makeShared<SequenceChunk>();
            sequenceChunk->insertAtEnd(contents);
            sequenceChunk->insertAtEnd(chunk);
            sequenceChunk->markImmutable();
            contents = sequenceChunk;
        }
        totalLength += chunk->getChunkLength();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removeFromBeginning(b length)
{
    CHUNK_CHECK_USAGE(b(0) <= length && length <= getTotalLength() - trailerIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(headerIterator.getPosition() == b(0) && (headerIterator.getIndex() == 0 || headerIterator.getIndex() == -1), "popped header length is non-zero");
    if (contents->getChunkLength() == length)
        contents = EmptyChunk::singleton;
    else if (contents->canRemoveFromBeginning(length)) {
        const auto& newContents = makeExclusivelyOwnedMutableChunk(contents);
        newContents->removeFromBeginning(length);
        newContents->markImmutable();
        contents = newContents;
    }
    else
        contents = contents->peek(length, contents->getChunkLength() - length);
    totalLength -= length;
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removeFromEnd(b length)
{
    CHUNK_CHECK_USAGE(b(0) <= length && length <= getTotalLength() - headerIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(trailerIterator.getPosition() == b(0) && (trailerIterator.getIndex() == 0 || trailerIterator.getIndex() == -1), "popped trailer length is non-zero");
    if (contents->getChunkLength() == length)
        contents = EmptyChunk::singleton;
    else if (contents->canRemoveFromEnd(length)) {
        const auto& newContents = makeExclusivelyOwnedMutableChunk(contents);
        newContents->removeFromEnd(length);
        newContents->markImmutable();
        contents = newContents;
    }
    else
        contents = contents->peek(b(0), contents->getChunkLength() - length);
    totalLength -= length;
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(headerIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(trailerIterator));
}

void Packet::removePoppedHeaders()
{
    b poppedLength = getHeaderPoppedLength();
    setHeaderPopOffset(b(0));
    removeFromBeginning(poppedLength);
}

void Packet::removePoppedTrailers()
{
    b poppedLength = getTrailerPoppedLength();
    setTrailerPopOffset(getTotalLength());
    removeFromEnd(poppedLength);
}

void Packet::removePoppedChunks()
{
    removePoppedHeaders();
    removePoppedTrailers();
}

void Packet::removeAll()
{
    contents = EmptyChunk::singleton;
    headerIterator = Chunk::ForwardIterator(b(0), 0);
    trailerIterator = Chunk::BackwardIterator(b(0), 0);
    totalLength = b(0);
    CHUNK_CHECK_IMPLEMENTATION(contents->isImmutable());
}

#if 0
// for descriptor:
// FIXME ez igy nem jo, mindig uj pointer keletkezik, az elozo pointer pedig ervenytelenne valik
const Chunk *Packet::getHeaderPart() const
{
    __headerCache = contents->peek(Chunk::ForwardIterator(b(0), 0), getHeaderPoppedLength());
    return __headerCache.get();
}

const Chunk *Packet::getDataPart() const
{
    __dataCache = peekData();
    return __dataCache.get();
}

const Chunk *Packet::getTrailerPart() const
{
    __trailerCache = contents->peek(Chunk::BackwardIterator(b(0), 0), getTrailerPoppedLength());
    return __trailerCache.get();
}
#endif

} // namespace

