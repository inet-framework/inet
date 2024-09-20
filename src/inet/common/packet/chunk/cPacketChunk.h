//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CPACKETCHUNK_H
#define __INET_CPACKETCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using an OMNeT++ cPacket instance. This can be
 * useful to make components using the new Packet class backward compatible
 * with other components using plain cPackets. The packet is owned by this
 * chunk and it shouldn't be deleted or modified in any way.
 */
class INET_API cPacketChunk : public Chunk
{
  protected:
    cPacket *packet;

  protected:
    const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    cPacketChunk(cPacket *packet = nullptr);
    cPacketChunk(const cPacketChunk& other);
    ~cPacketChunk() override;

    cPacketChunk *dup() const override { return new cPacketChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<cPacketChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    // TODO it should return a const cPacket *
    virtual cPacket *getPacket() const { return packet; }
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_CPACKET; }
    b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(packet->getBitLength() >= 0); return b(packet->getBitLength()); }

    bool containsSameData(const Chunk& other) const override;

    std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

