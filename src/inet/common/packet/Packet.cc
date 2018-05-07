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
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Register_Class(Packet);

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    content(EmptyChunk::singleton),
    frontIterator(Chunk::ForwardIterator(b(0), 0)),
    backIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(b(0))
{
    CHUNK_CHECK_IMPLEMENTATION(content->isImmutable());
}

Packet::Packet(const char *name, const Ptr<const Chunk>& content) :
    cPacket(name),
    content(content),
    frontIterator(Chunk::ForwardIterator(b(0), 0)),
    backIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(content->getChunkLength())
{
    constPtrCast<Chunk>(content)->markImmutable();
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    content(other.content),
    frontIterator(other.frontIterator),
    backIterator(other.backIterator),
    totalLength(other.totalLength),
    tags(other.tags)
{
    CHUNK_CHECK_IMPLEMENTATION(content->isImmutable());
}

void Packet::forEachChild(cVisitor *v)
{
    v->visit(const_cast<Chunk *>(content.get()));
}

void Packet::setFrontOffset(b offset)
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength() - backIterator.getPosition(), "offset is out of range");
    content->seekIterator(frontIterator, offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
}

const Ptr<const Chunk> Packet::peekAtFront(b length, int flags) const
{
    auto dataLength = getDataLength();
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
    const auto& chunk = content->peek(frontIterator, length, flags);
    if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
        return chunk;
    else
        return content->peek(frontIterator, dataLength, flags);
}

const Ptr<const Chunk> Packet::popAtFront(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekAtFront(length, flags);
    if (chunk != nullptr) {
        content->moveIterator(frontIterator, chunk->getChunkLength());
        CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
    }
    return chunk;
}

void Packet::setBackOffset(b offset)
{
    CHUNK_CHECK_USAGE(frontIterator.getPosition() <= offset, "offset is out of range");
    content->seekIterator(backIterator, getTotalLength() - offset);
    CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
}

const Ptr<const Chunk> Packet::peekAtBack(b length, int flags) const
{
    auto dataLength = getDataLength();
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
    const auto& chunk = content->peek(backIterator, length, flags);
    if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
        return chunk;
    else
        return content->peek(backIterator, dataLength, flags);
}

const Ptr<const Chunk> Packet::popAtBack(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    const auto& chunk = peekAtBack(length, flags);
    if (chunk != nullptr) {
        content->moveIterator(backIterator, chunk->getChunkLength());
        CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
    }
    return chunk;
}

const Ptr<const Chunk> Packet::peekDataAt(b offset, b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
    b peekOffset = frontIterator.getPosition() + offset;
    return content->peek(Chunk::Iterator(true, peekOffset, -1), length, flags);
}

const Ptr<const Chunk> Packet::peekAt(b offset, b length, int flags) const
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength(), "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getTotalLength(), "length is invalid");
    return content->peek(Chunk::Iterator(true, offset, -1), length, flags);
}

void Packet::insertAtBack(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(backIterator.getPosition() == b(0) && (backIterator.getIndex() == 0 || backIterator.getIndex() == -1), "popped trailer length is non-zero");
    constPtrCast<Chunk>(chunk)->markImmutable();
    if (content == EmptyChunk::singleton) {
        CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
        content = chunk;
        totalLength = content->getChunkLength();
    }
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
        }
        totalLength += chunk->getChunkLength();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(frontIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(backIterator));
}

void Packet::insertAtFront(const Ptr<const Chunk>& chunk)
{
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(frontIterator.getPosition() == b(0) && (frontIterator.getIndex() == 0 || frontIterator.getIndex() == -1), "popped header length is non-zero");
    constPtrCast<Chunk>(chunk)->markImmutable();
    if (content == EmptyChunk::singleton) {
        CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
        content = chunk;
        totalLength = content->getChunkLength();
    }
    else {
        if (content->canInsertAtFront(chunk)) {
            const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
            newContent->insertAtFront(chunk);
            newContent->markImmutable();
            content = newContent->simplify();
        }
        else {
            auto sequenceChunk = makeShared<SequenceChunk>();
            sequenceChunk->insertAtFront(content);
            sequenceChunk->insertAtFront(chunk);
            sequenceChunk->markImmutable();
            content = sequenceChunk;
        }
        totalLength += chunk->getChunkLength();
    }
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(frontIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(backIterator));
}

void Packet::eraseAtFront(b length)
{
    CHUNK_CHECK_USAGE(b(0) <= length && length <= getTotalLength() - backIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(frontIterator.getPosition() == b(0) && (frontIterator.getIndex() == 0 || frontIterator.getIndex() == -1), "popped header length is non-zero");
    if (content->getChunkLength() == length)
        content = EmptyChunk::singleton;
    else if (content->canRemoveAtFront(length)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAtFront(length);
        newContent->markImmutable();
        content = newContent;
    }
    else
        content = content->peek(length, content->getChunkLength() - length);
    totalLength -= length;
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(frontIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(backIterator));
}

void Packet::eraseAtBack(b length)
{
    CHUNK_CHECK_USAGE(b(0) <= length && length <= getTotalLength() - frontIterator.getPosition(), "length is invalid");
    CHUNK_CHECK_USAGE(backIterator.getPosition() == b(0) && (backIterator.getIndex() == 0 || backIterator.getIndex() == -1), "popped trailer length is non-zero");
    if (content->getChunkLength() == length)
        content = EmptyChunk::singleton;
    else if (content->canRemoveAtBack(length)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAtBack(length);
        newContent->markImmutable();
        content = newContent;
    }
    else
        content = content->peek(b(0), content->getChunkLength() - length);
    totalLength -= length;
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(frontIterator));
    CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(backIterator));
}

void Packet::eraseAll()
{
    content = EmptyChunk::singleton;
    frontIterator = Chunk::ForwardIterator(b(0), 0);
    backIterator = Chunk::BackwardIterator(b(0), 0);
    totalLength = b(0);
    CHUNK_CHECK_IMPLEMENTATION(content->isImmutable());
}

void Packet::trimFront()
{
    b length = frontIterator.getPosition();
    setFrontOffset(b(0));
    eraseAtFront(length);
}

void Packet::trimBack()
{
    b length = backIterator.getPosition();
    setBackOffset(getTotalLength());
    eraseAtBack(length);
}

void Packet::trim()
{
    trimFront();
    trimBack();
}

const Ptr<Chunk> Packet::removeAtFront(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(frontIterator.getPosition() == b(0), "popped header length is non-zero");
    const auto& chunk = popAtFront(length, flags);
    trimFront();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

const Ptr<Chunk> Packet::removeAtBack(b length, int flags)
{
    CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
    CHUNK_CHECK_USAGE(backIterator.getPosition() == b(0), "popped trailer length is non-zero");
    const auto& chunk = popAtBack(length, flags);
    trimBack();
    return makeExclusivelyOwnedMutableChunk(chunk);
}

const Ptr<Chunk> Packet::removeAll()
{
    const auto& oldContent = content;
    eraseAll();
    return makeExclusivelyOwnedMutableChunk(oldContent);
}

//(inet::Packet)UdpBasicAppData-0 (5000 bytes)
std::string Packet::str() const
{
    std::ostringstream out;
    out << "(" << getClassName() << ")" << getName() << " (" << getByteLength() << " bytes) [" << content->str() << "]";
    return out.str();
}

// TODO: move?
TagSet& getTags(cMessage *msg)
{
    if (msg->isPacket())
        return check_and_cast<Packet *>(msg)->getTags();
    else
        return check_and_cast<Message *>(msg)->getTags();
}

} // namespace

