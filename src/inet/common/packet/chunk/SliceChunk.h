//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SLICECHUNK_H
#define __INET_SLICECHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a slice of another chunk. The slice is
 * designated with the sliced chunk, an offset, and a length field, both
 * measured in bits. It's used by the Chunk API implementation internally to
 * efficiently represent slices of other chunks. User code should not directly
 * instantiate this class.
 */
class INET_API SliceChunk : public Chunk
{
    friend class Chunk;
    friend class SliceChunkDescriptor;

  protected:
    /**
     * The chunk of which this chunk is a slice, or nullptr if not yet specified.
     */
    Ptr<Chunk> chunk;
    /**
     * The offset measured in bits, or -1 if not yet specified.
     */
    b offset;
    /**
     * The length measured in bits, or -1 if not yet specified.
     */
    b length;

  protected:
    Chunk *_getChunk() const { return chunk.get(); } // only for class descriptor

    const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

    void doInsertAtFront(const Ptr<const Chunk>& chunk) override;
    void doInsertAtBack(const Ptr<const Chunk>& chunk) override;

    void doRemoveAtFront(b length) override;
    void doRemoveAtBack(b length) override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    SliceChunk();
    SliceChunk(const SliceChunk& other) = default;
    SliceChunk(const Ptr<Chunk>& chunk, b offset, b length);

    SliceChunk *dup() const override { return new SliceChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<SliceChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    void forEachChild(cVisitor *v) override;

    /** @name Field accessor functions */
    //@{
    const Ptr<Chunk>& getChunk() const { return chunk; }
    void setChunk(const Ptr<Chunk>& chunk) { CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable"); this->chunk = chunk; }

    b getOffset() const { return offset; }
    void setOffset(b offset);

    b getLength() const { return length; }
    void setLength(b length);
    //@}

    /** @name Overridden flag functions */
    //@{
    bool isMutable() const override { return Chunk::isMutable() || chunk->isMutable(); }
    bool isImmutable() const override { return Chunk::isImmutable() && chunk->isImmutable(); }

    bool isComplete() const override { return Chunk::isComplete() && chunk->isComplete(); }
    bool isIncomplete() const override { return Chunk::isIncomplete() || chunk->isIncomplete(); }

    bool isCorrect() const override { return Chunk::isCorrect() && chunk->isCorrect(); }
    bool isIncorrect() const override { return Chunk::isIncorrect() || chunk->isIncorrect(); }

    bool isProperlyRepresented() const override { return Chunk::isProperlyRepresented() && chunk->isProperlyRepresented(); }
    bool isImproperlyRepresented() const override { return Chunk::isImproperlyRepresented() || chunk->isImproperlyRepresented(); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_SLICE; }
    b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= b(0)); return length; }

    bool containsSameData(const Chunk& other) const override;

    bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override;
    bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override;

    bool canRemoveAtFront(b length) const override { return false; }
    bool canRemoveAtBack(b length) const override { return false; }

    std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

