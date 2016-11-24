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
    data(nullptr)
{
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    data(other.isImmutable() ? other.data : other.data->dupShared()),
    headerIterator(other.headerIterator),
    trailerIterator(other.trailerIterator)
{
}

Packet::Packet(const std::shared_ptr<Chunk>& data, const char *name, short kind) :
    cPacket(name, kind),
    data(data)
{
}

int Packet::getNumChunks() const
{
    if (data->getChunkType() == Chunk::TYPE_SEQUENCE)
        return std::static_pointer_cast<SequenceChunk>(data)->getChunks().size();
    else
        return 1;
}

Chunk *Packet::getChunk(int i) const
{
    if (data->getChunkType() == Chunk::TYPE_SEQUENCE)
        return std::static_pointer_cast<SequenceChunk>(data)->getChunks()[i].get();
    else
        return data.get();
}

std::shared_ptr<Chunk> Packet::peekHeader(int64_t length) const
{
    return data->peek(headerIterator, length);
}

std::shared_ptr<Chunk> Packet::peekHeaderAt(int64_t offset, int64_t length) const
{
    return data->peek(Chunk::Iterator(true, offset), length);
}

std::shared_ptr<Chunk> Packet::popHeader(int64_t length)
{
    const auto& chunk = peekHeader(length);
    if (chunk != nullptr)
        data->moveIterator(headerIterator, chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::peekTrailer(int64_t length) const
{
    return data->peek(trailerIterator, length);
}

std::shared_ptr<Chunk> Packet::peekTrailerAt(int64_t offset, int64_t length) const
{
    return data->peek(Chunk::Iterator(false, offset), length);
}

std::shared_ptr<Chunk> Packet::popTrailer(int64_t length)
{
    const auto& chunk = peekTrailer(length);
    if (chunk != nullptr)
        data->moveIterator(trailerIterator, -chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::peekData(int64_t length) const
{
    int64_t peekLength = length == -1 ? getDataLength() : length;
    return data->peek(Chunk::Iterator(true, getDataPosition()), peekLength);
}

std::shared_ptr<Chunk> Packet::peekDataAt(int64_t offset, int64_t length) const
{
    int64_t peekOffset = getDataPosition() + offset;
    int64_t peekLength = length == -1 ? getDataLength() - offset : length;
    return data->peek(Chunk::Iterator(true, peekOffset), peekLength);
}

std::shared_ptr<Chunk> Packet::peek(int64_t length) const
{
    int64_t peekLength = length == -1 ? getByteLength() : length;
    return data->peek(Chunk::Iterator(true, 0), peekLength);
}

std::shared_ptr<Chunk> Packet::peekAt(int64_t offset, int64_t length) const
{
    int64_t peekLength = length == -1 ? getByteLength() - offset : length;
    return data->peek(Chunk::Iterator(true, offset), peekLength);
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToBeginning(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (!data->insertToBeginning(chunk)) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToBeginning(data);
            sequenceChunk->insertToBeginning(chunk);
            data = sequenceChunk;
        }
    }
}

void Packet::append(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToEnd(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (!data->insertToEnd(chunk)) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->insertToEnd(data);
            sequenceChunk->insertToEnd(chunk);
            data = sequenceChunk;
        }
    }
}

void Packet::removeFromBeginning(int64_t length)
{
    if (!data->removeFromBeginning(length))
        throw cRuntimeError("Cannot remove from the beginning");
}

void Packet::removeFromEnd(int64_t length)
{
    if (!data->removeFromEnd(length))
        throw cRuntimeError("Cannot remove from the end");
}

} // namespace
