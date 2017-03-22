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

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

Register_Class(Packet);

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    contents(nullptr),
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
    assert(contents->isImmutable());
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
    if (contents == nullptr && offset == bit(0))
        return;
    else {
        assert(contents != nullptr);
        assert(bit(0) <= offset && offset <= getPacketLength() - trailerIterator.getPosition());
        contents->seekIterator(headerIterator, offset);
        assert(getDataLength() > bit(0));
    }
}

std::shared_ptr<Chunk> Packet::peekHeader(bit length, int flags) const
{
    assert(bit(-1) <= length && length <= getDataLength());
    return contents == nullptr ? nullptr : contents->peek(headerIterator, length, flags);
}

std::shared_ptr<Chunk> Packet::popHeader(bit length, int flags)
{
    assert(bit(-1) <= length && length <= getDataLength());
    const auto& chunk = peekHeader(length, flags);
    if (chunk != nullptr)
        contents->moveIterator(headerIterator, chunk->getChunkLength());
    return chunk;
}

void Packet::pushHeader(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(headerIterator.getPosition() == bit(0));
    prepend(chunk);
}

void Packet::setTrailerPopOffset(bit offset)
{
    if (contents == nullptr && offset == bit(0))
        return;
    else {
        assert(contents != nullptr);
        assert(headerIterator.getPosition() <= offset);
        contents->seekIterator(trailerIterator, getPacketLength() - offset);
        assert(getDataLength() > bit(0));
    }
}

std::shared_ptr<Chunk> Packet::peekTrailer(bit length, int flags) const
{
    assert(bit(-1) <= length && length <= getDataLength());
    return contents == nullptr ? nullptr : contents->peek(trailerIterator, length, flags);
}

std::shared_ptr<Chunk> Packet::popTrailer(bit length, int flags)
{
    assert(bit(-1) <= length && length <= getDataLength());
    const auto& chunk = peekTrailer(length, flags);
    if (chunk != nullptr)
        contents->moveIterator(trailerIterator, -chunk->getChunkLength());
    return chunk;
}

void Packet::pushTrailer(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(trailerIterator.getPosition() == bit(0));
    append(chunk);
}

std::shared_ptr<Chunk> Packet::peekDataAt(bit offset, bit length, int flags) const
{
    assert(bit(0) <= offset && offset <= getDataLength());
    assert(bit(-1) <= length && length <= getDataLength());
    if (contents == nullptr)
        return nullptr;
    else {
        bit peekOffset = headerIterator.getPosition() + offset;
        bit peekLength = length == bit(-1) ? getDataLength() - offset : length;
        return contents->peek(Chunk::Iterator(true, peekOffset, -1), peekLength, flags);
    }
}

std::shared_ptr<Chunk> Packet::peekAt(bit offset, bit length, int flags) const
{
    assert(bit(0) <= offset && offset <= getPacketLength());
    assert(bit(-1) <= length && length <= getPacketLength());
    if (contents == nullptr)
        return nullptr;
    else {
        bit peekLength = length == bit(-1) ? getPacketLength() - offset : length;
        return contents->peek(Chunk::Iterator(true, offset, -1), peekLength, flags);
    }
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    assert(headerIterator.getPosition() == bit(0));
    if (contents == nullptr)
        contents = chunk;
    else {
        if (contents->canInsertAtBeginning(chunk)) {
            makeContentsMutable();
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
}

void Packet::append(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    assert(trailerIterator.getPosition() == bit(0));
    if (contents == nullptr)
        contents = chunk;
    else {
        if (contents->canInsertAtEnd(chunk)) {
            makeContentsMutable();
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
}

void Packet::removeFromBeginning(bit length)
{
    if (contents == nullptr && length == bit(0))
        return;
    else {
        assert(bit(0) <= length && length <= getPacketLength());
        assert(contents != nullptr);
        assert(headerIterator.getPosition() == bit(0));
        if (contents->getChunkLength() == length)
            contents = nullptr;
        else if (contents->canRemoveFromBeginning(length)) {
            makeContentsMutable();
            contents->removeFromBeginning(length);
        }
        else
            contents = contents->peek(length, contents->getChunkLength() - length, Chunk::PF_ALLOW_NULLPTR);
        if (contents != nullptr)
            contents->markImmutable();
    }
}

void Packet::removeFromEnd(bit length)
{
    if (contents == nullptr && length == bit(0))
        return;
    else {
        assert(bit(0) <= length && length <= getPacketLength());
        assert(contents != nullptr);
        assert(trailerIterator.getPosition() == bit(0));
        if (contents->getChunkLength() == length)
            contents = nullptr;
        else if (contents->canRemoveFromEnd(length)) {
            makeContentsMutable();
            contents->removeFromEnd(length);
        }
        else
            contents = contents->peek(bit(0), contents->getChunkLength() - length, Chunk::PF_ALLOW_NULLPTR);
        if (contents != nullptr)
            contents->markImmutable();
    }
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
    setTrailerPopOffset(getPacketLength());
    removeFromEnd(poppedLength);
}

void Packet::removePoppedChunks()
{
    removePoppedHeaders();
    removePoppedTrailers();
}

} // namespace
