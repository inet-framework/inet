//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BITCOUNTCHUNK_H
#define __INET_BITCOUNTCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a bit length field only. This can be useful
 * when the actual data is irrelevant and memory efficiency is high priority.
 */
class INET_API BitCountChunk : public Chunk
{
    friend class Chunk;

  protected:
    /**
     * The chunk length in bits, or -1 if not yet specified.
     */
    b length;
    bool data;

  protected:
    const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

    void doInsertAtFront(const Ptr<const Chunk>& chunk) override;
    void doInsertAtBack(const Ptr<const Chunk>& chunk) override;
    void doInsertAt(const Ptr<const Chunk>& chunk, b offset) override;

    void doRemoveAtFront(b length) override;
    void doRemoveAtBack(b length) override;
    void doRemoveAt(b offset, b length) override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    BitCountChunk();
    BitCountChunk(const BitCountChunk& other) = default;
    BitCountChunk(b length, bool data = false);

    BitCountChunk *dup() const override { return new BitCountChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<BitCountChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    b getLength() const { return length; }
    void setLength(b length);

    bool getData() const { return data; }
    void setData(bool data);
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_BITCOUNT; }
    b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= b(0)); return length; }

    bool containsSameData(const Chunk& other) const override;

    bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override;
    bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override;
    bool canInsertAt(const Ptr<const Chunk>& chunk, b offset) const override;

    bool canRemoveAtFront(b length) const override { return true; }
    bool canRemoveAtBack(b length) const override { return true; }
    bool canRemoveAt(b offset, b length) const override { return true; }

    std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

