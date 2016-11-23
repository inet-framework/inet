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

std::shared_ptr<Chunk> Packet::peekHeader(int64_t byteLength) const
{
    return data->peek(headerIterator, byteLength);
}

std::shared_ptr<Chunk> Packet::peekHeaderAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(Chunk::Iterator(true, byteOffset), byteLength);
}

std::shared_ptr<Chunk> Packet::popHeader(int64_t byteLength)
{
    const auto& chunk = peekHeader(byteLength);
    if (chunk != nullptr)
        data->moveIterator(headerIterator, chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::peekTrailer(int64_t byteLength) const
{
    return data->peek(trailerIterator, byteLength);
}

std::shared_ptr<Chunk> Packet::peekTrailerAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(Chunk::Iterator(false, byteOffset), byteLength);
}

std::shared_ptr<Chunk> Packet::popTrailer(int64_t byteLength)
{
    const auto& chunk = peekTrailer(byteLength);
    if (chunk != nullptr)
        data->moveIterator(trailerIterator, -chunk->getChunkLength());
    return chunk;
}

std::shared_ptr<Chunk> Packet::peekData(int64_t byteLength) const
{
    int64_t peekByteLength = byteLength == -1 ? getDataLength() : byteLength;
    return data->peek(Chunk::Iterator(true, getDataPosition()), peekByteLength);
}

std::shared_ptr<Chunk> Packet::peekDataAt(int64_t byteOffset, int64_t byteLength) const
{
    int64_t peekByteOffset = getDataPosition() + byteOffset;
    int64_t peekByteLength = byteLength == -1 ? getDataLength() - byteOffset : byteLength;
    return data->peek(Chunk::Iterator(true, peekByteOffset), peekByteLength);
}

std::shared_ptr<Chunk> Packet::peek(int64_t byteLength) const
{
    int64_t peekByteLength = byteLength == -1 ? getByteLength() : byteLength;
    return data->peek(Chunk::Iterator(true, 0), peekByteLength);
}

std::shared_ptr<Chunk> Packet::peekAt(int64_t byteOffset, int64_t byteLength) const
{
    int64_t peekByteLength = byteLength == -1 ? getByteLength() - byteOffset : byteLength;
    return data->peek(Chunk::Iterator(true, byteOffset), peekByteLength);
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->append(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (data->getChunkType() == Chunk::TYPE_SEQUENCE)
            std::static_pointer_cast<SequenceChunk>(data)->prepend(chunk);
        else {
            if (!data->insertToBeginning(chunk)) {
                auto sequenceChunk = std::make_shared<SequenceChunk>();
                sequenceChunk->prepend(data);
                sequenceChunk->prepend(chunk);
                data = sequenceChunk;
            }
        }
    }
}

void Packet::append(const std::shared_ptr<Chunk>& chunk)
{
    if (data == nullptr) {
        if (chunk->getChunkType() == Chunk::TYPE_SLICE) {
            auto sequenceChunk = std::make_shared<SequenceChunk>();
            sequenceChunk->append(chunk);
            data = sequenceChunk;
        }
        else
            data = chunk;
    }
    else {
        if (data->getChunkType() == Chunk::TYPE_SEQUENCE)
            std::static_pointer_cast<SequenceChunk>(data)->append(chunk);
        else {
            if (!data->insertToEnd(chunk)) {
                auto sequenceChunk = std::make_shared<SequenceChunk>();
                sequenceChunk->append(data);
                sequenceChunk->append(chunk);
                data = sequenceChunk;
            }
        }
    }
}

void Packet::removeFromBeginning(int64_t byteLength)
{
    if (!data->removeFromBeginning(byteLength))
        throw cRuntimeError("Cannot remove from the beginning");
}

void Packet::removeFromEnd(int64_t byteLength)
{
    if (!data->removeFromEnd(byteLength))
        throw cRuntimeError("Cannot remove from the end");
}

} // namespace
