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
#include "inet/common/packet/SequenceChunk.h"

namespace inet {

Register_Class(Packet);

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    contents(nullptr),
    headerIterator(Chunk::ForwardIterator(0, 0)),
    trailerIterator(Chunk::BackwardIterator(0, 0))
{
}

Packet::Packet(const char *name, const std::shared_ptr<Chunk>& contents) :
    cPacket(name),
    contents(contents),
    headerIterator(Chunk::ForwardIterator(0, 0)),
    trailerIterator(Chunk::BackwardIterator(0, 0))
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

int Packet::getNumChunks() const
{
    if (contents == nullptr)
        return 0;
    else if (contents->getChunkType() == Chunk::TYPE_SEQUENCE)
        return std::static_pointer_cast<SequenceChunk>(contents)->getChunks().size();
    else
        return 1;
}

Chunk *Packet::getChunk(int i) const
{
    assert(contents != nullptr);
    assert(0 <= i && i < getNumChunks());
    if (contents->getChunkType() == Chunk::TYPE_SEQUENCE)
        return std::static_pointer_cast<SequenceChunk>(contents)->getChunks()[i].get();
    else
        return contents.get();
}

void Packet::setHeaderPopOffset(int64_t offset)
{
    assert(contents != nullptr);
    assert(0 <= offset && offset <= getPacketLength() - trailerIterator.getPosition());
    contents->seekIterator(headerIterator, offset);
    assert(getDataLength() > 0);
}

std::shared_ptr<Chunk> Packet::peekHeader(int64_t length) const
{
    assert(-1 <= length && length <= getDataLength());
    return contents == nullptr ? nullptr : contents->peek(headerIterator, length);
}

std::shared_ptr<Chunk> Packet::popHeader(int64_t length)
{
    assert(-1 <= length && length <= getDataLength());
    const auto& chunk = peekHeader(length);
    if (chunk != nullptr)
        contents->moveIterator(headerIterator, chunk->getChunkLength());
    return chunk;
}

void Packet::pushHeader(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(headerIterator.getPosition() == 0);
    prepend(chunk);
}

void Packet::setTrailerPopOffset(int64_t offset)
{
    assert(contents != nullptr);
    assert(headerIterator.getPosition() <= offset);
    contents->seekIterator(trailerIterator, getPacketLength() - offset);
    assert(getDataLength() > 0);
}

std::shared_ptr<Chunk> Packet::peekTrailer(int64_t length) const
{
    assert(-1 <= length && length <= getDataLength());
    return contents == nullptr ? nullptr : contents->peek(trailerIterator, length);
}

std::shared_ptr<Chunk> Packet::popTrailer(int64_t length)
{
    assert(-1 <= length && length <= getDataLength());
    const auto& chunk = peekTrailer(length);
    if (chunk != nullptr)
        contents->moveIterator(trailerIterator, -chunk->getChunkLength());
    return chunk;
}

void Packet::pushTrailer(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(trailerIterator.getPosition() == 0);
    append(chunk);
}

std::shared_ptr<Chunk> Packet::peekDataAt(int64_t offset, int64_t length) const
{
    assert(0 <= offset && offset <= getDataLength());
    assert(-1 <= length && length <= getDataLength());
    if (contents == nullptr)
        return nullptr;
    else {
        int64_t peekOffset = headerIterator.getPosition() + offset;
        int64_t peekLength = length == -1 ? getDataLength() - offset : length;
        return contents->peek(Chunk::Iterator(true, peekOffset, -1), peekLength);
    }
}

std::shared_ptr<Chunk> Packet::peekAt(int64_t offset, int64_t length) const
{
    assert(0 <= offset && offset <= getPacketLength());
    assert(-1 <= length && length <= getPacketLength());
    if (contents == nullptr)
        return nullptr;
    else {
        int64_t peekLength = length == -1 ? getByteLength() - offset : length;
        return contents->peek(Chunk::Iterator(true, offset, -1), peekLength);
    }
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk)
{
    assert(chunk != nullptr);
    assert(chunk->isImmutable());
    assert(headerIterator.getPosition() == 0);
    if (contents == nullptr)
        contents = chunk;
    else {
        if (contents->canInsertAtBeginning(chunk)) {
            makeContentsMutable();
            contents->insertAtBeginning(chunk);
            contents = contents->peek(0, contents->getChunkLength());
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
    assert(trailerIterator.getPosition() == 0);
    if (contents == nullptr)
        contents = chunk;
    else {
        if (contents->canInsertAtEnd(chunk)) {
            makeContentsMutable();
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

void Packet::removeFromBeginning(int64_t length)
{
    assert(0 <= length && length <= getPacketLength());
    assert(headerIterator.getPosition() == 0);
    if (contents->canRemoveFromBeginning(length)) {
        makeContentsMutable();
        contents->removeFromBeginning(length);
    }
    else
        contents = contents->peek(length, contents->getChunkLength() - length);
    contents->markImmutable();
}

void Packet::removeFromEnd(int64_t length)
{
    assert(0 <= length && length <= getPacketLength());
    assert(trailerIterator.getPosition() == 0);
    if (contents->canRemoveFromEnd(length)) {
        makeContentsMutable();
        contents->removeFromEnd(length);
    }
    else
        contents = contents->peek(0, contents->getChunkLength() - length);
    contents->markImmutable();
}

} // namespace
