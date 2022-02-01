//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BYTECOUNTCHUNK_H
#define __INET_BYTECOUNTCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a byte length field only. This can be useful
 * when the actual data is irrelevant and memory efficiency is high priority.
 */
class INET_API ByteCountChunk : public Chunk
{
    friend class Chunk;

  protected:
    /**
     * The chunk length in bytes, or -1 if not yet specified.
     */
    B length;
    uint8_t data;

  protected:
    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

    virtual void doInsertAtFront(const Ptr<const Chunk>& chunk) override;
    virtual void doInsertAtBack(const Ptr<const Chunk>& chunk) override;
    virtual void doInsertAt(const Ptr<const Chunk>& chunk, b offset) override;

    virtual void doRemoveAtFront(b length) override;
    virtual void doRemoveAtBack(b length) override;
    virtual void doRemoveAt(b offset, b length) override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ByteCountChunk();
    ByteCountChunk(const ByteCountChunk& other) = default;
    ByteCountChunk(B length, uint8_t data = '?');

    virtual ByteCountChunk *dup() const override { return new ByteCountChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<ByteCountChunk>(*this); }

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    B getLength() const { return length; }
    void setLength(B length);

    uint8_t getData() const { return data; }
    void setData(uint8_t data);
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_BYTECOUNT; }
    virtual b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= B(0)); return length; }

    virtual bool containsSameData(const Chunk& other) const override;

    virtual bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override;
    virtual bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override;
    virtual bool canInsertAt(const Ptr<const Chunk>& chunk, b offset) const override;

    virtual bool canRemoveAtFront(b length) const override { return b(length).get() % 8 == 0; }
    virtual bool canRemoveAtBack(b length) const override { return b(length).get() % 8 == 0; }
    virtual bool canRemoveAt(b offset, b length) const override { return b(offset).get() % 8 == 0 && b(length).get() % 8 == 0; }

    virtual std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

