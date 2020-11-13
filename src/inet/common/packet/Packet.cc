//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/packet/Packet.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/dissector/PacketDissector.h"

namespace inet {

Register_Class(Packet);

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    content(EmptyChunk::singleton),
    frontIterator(Chunk::ForwardIterator(b(0), 0)),
    backIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(b(0))
{
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

Packet::Packet(const char *name, const Ptr<const Chunk>& content) :
    cPacket(name),
    content(content),
    frontIterator(Chunk::ForwardIterator(b(0), 0)),
    backIterator(Chunk::BackwardIterator(b(0), 0)),
    totalLength(content->getChunkLength())
{
    constPtrCast<Chunk>(content)->markImmutable();
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    content(other.content),
    frontIterator(other.frontIterator),
    backIterator(other.backIterator),
    totalLength(other.totalLength),
    tags(other.tags),
    regionTags(other.regionTags)
{
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

// Class descriptor functions

const ChunkTemporarySharedPtr *Packet::getDissection() const
{
    PacketDissector::ChunkBuilder builder;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, builder);
    packetDissector.dissectPacket(const_cast<Packet *>(this));
    return new ChunkTemporarySharedPtr(builder.getContent());
}

const ChunkTemporarySharedPtr *Packet::getFront() const
{
    const auto& chunk = peekAt(b(0), getFrontOffset(), Chunk::PF_ALLOW_ALL);
    return chunk == nullptr? nullptr : new ChunkTemporarySharedPtr(chunk);
}

const ChunkTemporarySharedPtr *Packet::getData() const
{
    const auto& chunk = peekData(Chunk::PF_ALLOW_ALL);
    return chunk == nullptr? nullptr : new ChunkTemporarySharedPtr(chunk);
}

const ChunkTemporarySharedPtr *Packet::getBack() const
{
    const auto& chunk = peekAt(getBackOffset(), backIterator.getPosition(), Chunk::PF_ALLOW_ALL);
    return chunk == nullptr? nullptr : new ChunkTemporarySharedPtr(chunk);
}

// Supported cPacket interface functions

void Packet::forEachChild(cVisitor *v)
{
    cPacket::forEachChild(v);
    v->visit(const_cast<Chunk *>(content.get()));
    for (int i = 0; i < tags.getNumTags(); i++)
        v->visit(const_cast<TagBase *>(tags.getTag(i).get()));
    for (int i = 0; i < regionTags.getNumTags(); i++)
        v->visit(const_cast<TagBase *>(regionTags.getTag(i).get()));
}

void Packet::parsimPack(cCommBuffer *buffer) const
{
    cPacket::parsimPack(buffer);
    buffer->packObject(const_cast<Chunk *>(content.get()));
    buffer->pack(frontIterator.getIndex());
    buffer->pack(frontIterator.getPosition().get());
    buffer->pack(backIterator.getIndex());
    buffer->pack(backIterator.getPosition().get());
    tags.parsimPack(buffer);
    regionTags.parsimPack(buffer);
}

void Packet::parsimUnpack(cCommBuffer *buffer)
{
    cPacket::parsimUnpack(buffer);
    content = check_and_cast<Chunk *>(buffer->unpackObject())->shared_from_this();
    totalLength = content->getChunkLength();
    int index;
    buffer->unpack(index);
    frontIterator.setIndex(index);
    uint64_t position;
    buffer->unpack(position);
    frontIterator.setPosition(b(position));
    buffer->unpack(index);
    backIterator.setIndex(index);
    buffer->unpack(position);
    backIterator.setPosition(b(position));
    tags.parsimUnpack(buffer);
    regionTags.parsimUnpack(buffer);
}

// Front and back offset related functions

void Packet::setFrontOffset(b offset)
{
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength() - backIterator.getPosition(), "offset is out of range");
    content->seekIterator(frontIterator, offset);
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

void Packet::setBackOffset(b offset)
{
    CHUNK_CHECK_USAGE(frontIterator.getPosition() <= offset && offset <= getTotalLength(), "offset is out of range");
    content->seekIterator(backIterator, getTotalLength() - offset);
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

// Content insertion functions

void Packet::insertAt(const Ptr<const Chunk>& chunk, b offset)
{
    auto totalLength = getTotalLength();
    CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
    CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
    constPtrCast<Chunk>(chunk)->markImmutable();
    if (content == EmptyChunk::singleton)
        content = chunk;
    else if (offset == b(0) && content->canInsertAtFront(chunk)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->insertAtFront(chunk);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else if (offset == totalLength && content->canInsertAtBack(chunk)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->insertAtBack(chunk);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else if (content->canInsertAt(chunk, offset)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->insertAt(chunk, offset);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else {
        auto sequenceChunk = makeShared<SequenceChunk>();
        if (offset != b(0))
            sequenceChunk->insertAtBack(content->peek(Chunk::ForwardIterator(b(0), 0), offset, Chunk::PF_ALLOW_ALL));
        sequenceChunk->insertAtBack(chunk);
        if (offset != totalLength)
            sequenceChunk->insertAtBack(content->peek(Chunk::ForwardIterator(offset, -1), totalLength, Chunk::PF_ALLOW_ALL));
        sequenceChunk->markImmutable();
        content = sequenceChunk;
    }
    b chunkLength = chunk->getChunkLength();
    if (offset < getFrontOffset())
        content->moveIterator(frontIterator, chunkLength);
    if (offset > getBackOffset())
        content->moveIterator(backIterator, chunkLength);
    this->totalLength += chunkLength;
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
}

// Content erasing functions

void Packet::eraseAt(b offset, b length)
{
    auto totalLength = getTotalLength();
    CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
    CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
    if (content->getChunkLength() == length)
        content = EmptyChunk::singleton;
    else if (offset == b(0) && content->canRemoveAtFront(length)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAtFront(length);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else if (offset + length == totalLength && content->canRemoveAtBack(length))  {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAtBack(length);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else if (content->canRemoveAt(offset, length)) {
        const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
        newContent->removeAt(offset, offset);
        newContent->markImmutable();
        content = newContent->simplify();
    }
    else {
        auto sequenceChunk = makeShared<SequenceChunk>();
        if (offset != b(0))
            sequenceChunk->insertAtBack(content->peek(Chunk::ForwardIterator(b(0), 0), offset));
        if (offset != totalLength)
            sequenceChunk->insertAtBack(content->peek(Chunk::ForwardIterator(offset + length, -1), totalLength - length));
        sequenceChunk->markImmutable();
        content = sequenceChunk;
    }
    b frontEraseLength = getFrontOffset() - offset;
    b backEraseLength = offset + length - getBackOffset();
    if (frontEraseLength > b(0))
        content->moveIterator(frontIterator, -frontEraseLength);
    if (backEraseLength > b(0))
        content->moveIterator(backIterator, -backEraseLength);
    this->totalLength -= length;
    CHUNK_CHECK_IMPLEMENTATION(isConsistent());
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

// Utility functions

const char *Packet::getFullName() const
{
    if (!isUpdate())
        return getName();
    else {
        const char *suffix;
        if (getRemainingDuration().isZero())
            suffix = ":end";
        else if (getRemainingDuration() == getDuration())
            suffix = ":start";
        else
            suffix = ":progress";
        static std::set<std::string> pool;
        std::string fullname = std::string(getName()) + suffix;
        auto it = pool.insert(fullname).first;
        return it->c_str();
    }
}

std::ostream& Packet::printToStream(std::ostream& stream, int level, int evFlags) const
{
    std::string className = getClassName();
    auto index = className.rfind("::");
    if (index != std::string::npos)
        className = className.substr(index + 2);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FAINT << "(" << className << ")" << EV_NORMAL;
    stream << EV_ITALIC << getName() << EV_NORMAL << " (" << getDataLength() << ") ";
    content->printToStream(stream, level + 1, evFlags);
    return stream;
}

std::string Packet::str() const
{
    std::stringstream stream;
    stream << "(" << getDataLength() << ") " << content;
    return stream.str();
}

} // namespace

