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
    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    cPacketChunk(cPacket *packet = nullptr);
    cPacketChunk(const cPacketChunk& other);
    ~cPacketChunk();

    virtual cPacketChunk *dup() const override { return new cPacketChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<cPacketChunk>(*this); }

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    // TODO it should return a const cPacket *
    virtual cPacket *getPacket() const { return packet; }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_CPACKET; }
    virtual b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(packet->getBitLength() >= 0); return b(packet->getBitLength()); }

    virtual bool containsSameData(const Chunk& other) const override;

    virtual std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

