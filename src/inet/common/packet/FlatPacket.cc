//TODO header

#include <algorithm>

#include "inet/common/packet/FlatPacket.h"

namespace inet {

Chunk::Chunk(const char *name, bool namepooling)
    :cOwnedObject(name, namepooling)
{
}

Chunk::Chunk(const Chunk& other) : ::omnetpp::cOwnedObject(other)
{
    copy(other);
}

Chunk::~Chunk()
{
}

void Chunk::copy(const Chunk& other)
{
    chunkBitLength = other.chunkBitLength;
}

void Chunk::addChunkBitLength(int64_t x)
{
//    if (getOwnerPacket() != nullptr)
//        throw cRuntimeError("setChunkBitLength(): PacketChunk Owned by a FlatPacket. Should remove PacketChunk from FlatPacket before modify content.");
    chunkBitLength += x;
    if (chunkBitLength < 0)
        throw cRuntimeError(this, "addChunkBitLength(): length became negative (%" INT64_PRINTF_FORMAT ") after adding %" INT64_PRINTF_FORMAT "d", chunkBitLength, x);
    if (FlatPacket *pk = getOwnerPacket()) {
        pk->addBitLength(x);
    }
}

void Chunk::setChunkBitLength(int64_t x)
{
//    if (getOwnerPacket() != nullptr)
//        throw cRuntimeError("setChunkBitLength(): PacketChunk Owned by a FlatPacket. Should remove PacketChunk from FlatPacket before modify content.");
    if (FlatPacket *pk = getOwnerPacket()) {
        pk->addBitLength(x - chunkBitLength);
    }
    chunkBitLength = x;
}

FlatPacket *Chunk::getOwnerPacket() const
{
    cObject *owner = getOwner();
    if ((owner != nullptr) && (typeid(*owner) == typeid(FlatPacket)))
        return static_cast<FlatPacket *>(owner);
    return nullptr;
}

FlatPacket *Chunk::getMandatoryOwnerPacket() const
{
    FlatPacket *fp = getOwnerPacket();
    if (fp == nullptr)
        throw cRuntimeError("Chunk does not owned by a FlatPacket");
    else
        return fp;
}

///////////////////////////////////////////////////////////////////////////

PacketChunk::PacketChunk(cPacket *pk)
    : packet(pk)
{
    take(packet);
    setChunkBitLength(pk->getBitLength());
}

PacketChunk::PacketChunk(const PacketChunk& other) : Chunk(other)
{
    copy(other);
}

PacketChunk::~PacketChunk()
{
    dropAndDelete(packet);
}

void PacketChunk::copy(const PacketChunk& other)
{
    packet = other.packet->dup();
    take(packet);
}

cPacket *PacketChunk::removePacket()
{
    if (getOwnerPacket() != nullptr)           // throw error when PacketChunk owned by a FlatPacket
        throw cRuntimeError("removePacket(): PacketChunk Owned by a FlatPacket. Should remove PacketChunk from FlatPacket before modifying content");
    cPacket *pk = packet;
    packet = nullptr;
    drop(pk);
    return pk;
}

void PacketChunk::setPacket(cPacket *pk)
{
    if (packet != nullptr)           // throw error when PacketChunk already contains a packet
        throw cRuntimeError("setPacket(): PacketChunk already owns another packet.");
    if (getOwnerPacket() != nullptr)           // throw error when PacketChunk owned by a FlatPacket
        throw cRuntimeError("setPacket(): PacketChunk Owned by a FlatPacket. Should remove PacketChunk from FlatPacket before modifying content");
    packet = pk;
    setChunkBitLength(pk->getBitLength());
    take(packet);
}

int64_t PacketChunk::getChunkBitLength() const
{
    return (packet != nullptr) ? packet->getBitLength() : 0;
}

///////////////////////////////////////////////////////////////////////////

FlatPacket::FlatPacket(const char *name, short kind, int64_t bitLength)
    : cPacket(name, kind, bitLength)
{
}

FlatPacket::FlatPacket(const FlatPacket& other) : cPacket(other)
{
    copy(other);
}

FlatPacket::~FlatPacket()
{
    for (Chunk *chunk: chunks) {
        dropAndDelete(chunk);
    }
}

void FlatPacket::copy(const FlatPacket& other)
{
    for (Chunk *chunk: other.chunks) {
        Chunk *chunkCopy = chunk->dup();
        take(chunkCopy);
        chunks.push_back(chunkCopy);
    }
}

void FlatPacket::pushHeader(Chunk *chunk)
{
    take(chunk);
    chunks.insert(chunks.begin(), chunk);
    cPacket::addBitLength(chunk->getChunkBitLength());
}

void FlatPacket::pushTrailer(Chunk *chunk)
{
    take(chunk);
    chunks.push_back(chunk);
    cPacket::addBitLength(chunk->getChunkBitLength());
}

Chunk *FlatPacket::peekHeader()
{
    return chunks.empty() ? nullptr : chunks.front();
}

const Chunk *FlatPacket::peekHeader() const
{
    return chunks.empty() ? nullptr : chunks.front();
}

Chunk *FlatPacket::peekTrailer()
{
    return chunks.empty() ? nullptr : chunks.back();
}

const Chunk *FlatPacket::peekTrailer() const
{
    return chunks.empty() ? nullptr : chunks.back();
}

Chunk *FlatPacket::popHeader()
{
    if (chunks.empty())
        return nullptr;
    Chunk *chunk = chunks.front();
    chunks.erase(chunks.begin());
    drop(chunk);
    cPacket::addBitLength(-chunk->getChunkBitLength());
    return chunk;
}

Chunk *FlatPacket::popTrailer()
{
    if (chunks.empty())
        return nullptr;
    Chunk *chunk = chunks.back();
    chunks.pop_back();
    drop(chunk);
    cPacket::addBitLength(-chunk->getChunkBitLength());
    return chunk;
}

int FlatPacket::getNumChunks() const
{
    return chunks.size();
}

Chunk *FlatPacket::getChunk(int i)
{
    ASSERT(i>=0 && i<chunks.size());
    return chunks.at(i);
}

const Chunk *FlatPacket::getChunk(int i) const
{
    ASSERT(i>=0 && i<chunks.size());
    return chunks.at(i);
}

int FlatPacket::getChunkIndex(const Chunk *chunk) const
{
    //FIXME use std::find
    auto it = std::find(chunks.begin(), chunks.end(), chunk);
    return it == chunks.end() ? -1 : std::distance(chunks.begin(), it);
}

int64_t FlatPacket::getBitLength() const
{
    //FIXME use cache: should be stored length in cPacket::length field
#if 1
    int64_t length = 0;
    for (auto chunk: chunks)
        length += chunk->getChunkBitLength();
    ASSERT(length == cPacket::getBitLength());
    return length;
#else
    return cPacket::getBitLength();
#endif
}

}   //namespace inet
