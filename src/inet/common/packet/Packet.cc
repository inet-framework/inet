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

#include "Packet.h"

namespace inet {

Packet::Packet(const char *name, short kind) :
    cPacket(name, kind),
    data(std::make_shared<SequenceChunk>()),
    headerIterator(data->createForwardIterator()),
    trailerIterator(data->createBackwardIterator())
{
}

Packet::Packet(const Packet& other) :
    cPacket(other),
    data(other.isImmutable() ? other.data : std::make_shared<SequenceChunk>(*other.data)),
    headerIterator(other.headerIterator),
    trailerIterator(other.trailerIterator)
{
}

int Packet::getNumChunks() const
{
    return data->chunks.size();
}

Chunk *Packet::getChunk(int i) const
{
    return data->chunks[i].get();
}

std::shared_ptr<Chunk> Packet::peekHeader(int64_t byteLength) const
{
    return data->peek(headerIterator, byteLength);
}

std::shared_ptr<Chunk> Packet::peekHeaderAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(SequenceChunk::ForwardIterator(data, -1, byteOffset), byteLength);
}

std::shared_ptr<Chunk> Packet::peekTrailer(int64_t byteLength) const
{
    return data->peek(trailerIterator, byteLength);
}

std::shared_ptr<Chunk> Packet::peekTrailerAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(SequenceChunk::BackwardIterator(data, -1, byteOffset), byteLength);
}

std::shared_ptr<Chunk> Packet::peekData(int64_t byteLength) const
{
    return data->peek(SequenceChunk::ForwardIterator(data, -1, getDataPosition()), byteLength == -1 ? getDataSize() : byteLength);
}

std::shared_ptr<Chunk> Packet::peekDataAt(int64_t byteOffset, int64_t byteLength) const
{
    return data->peek(SequenceChunk::ForwardIterator(data, -1, byteOffset), byteLength);
}

void Packet::prepend(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    data->prepend(chunk, flatten);
}

void Packet::prepend(Packet* packet, bool flatten)
{
    data->prepend(packet->data, flatten);
}

void Packet::append(const std::shared_ptr<Chunk>& chunk, bool flatten)
{
    data->append(chunk, flatten);
}

void Packet::append(Packet* packet, bool flatten)
{
    data->append(packet->data, flatten);
}

} // namespace
