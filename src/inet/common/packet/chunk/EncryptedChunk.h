//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENCRYPTEDCHUNK_H
#define __INET_ENCRYPTEDCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents encrypted data of another chunk. The original data is
 * specified with another chunk.
 */
class INET_API EncryptedChunk : public Chunk
{
    friend class Chunk;
    friend class EncryptedChunkDescriptor;

  protected:
    /**
     * The chunk that is encrypted by this chunk, or nullptr if not yet specified.
     */
    Ptr<Chunk> chunk;
    /**
     * The encrypted length measured in bits, or -1 if not yet specified.
     */
    b length;

  protected:
    Chunk *_getChunk() const { return chunk.get(); } // only for class descriptor

    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    EncryptedChunk();
    EncryptedChunk(const EncryptedChunk& other) = default;
    EncryptedChunk(const Ptr<Chunk>& chunk, b length);

    virtual EncryptedChunk *dup() const override { return new EncryptedChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<EncryptedChunk>(*this); }

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    virtual void forEachChild(cVisitor *v) override;

    /** @name Field accessor functions */
    //@{
    const Ptr<Chunk>& getChunk() const { return chunk; }
    void setChunk(const Ptr<Chunk>& chunk) { CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable"); this->chunk = chunk; }

    b getLength() const { return length; }
    void setLength(b length);
    //@}

    /** @name Overridden flag functions */
    //@{
    virtual bool isMutable() const override { return Chunk::isMutable() || chunk->isMutable(); }
    virtual bool isImmutable() const override { return Chunk::isImmutable() && chunk->isImmutable(); }

    virtual bool isComplete() const override { return Chunk::isComplete() && chunk->isComplete(); }
    virtual bool isIncomplete() const override { return Chunk::isIncomplete() || chunk->isIncomplete(); }

    virtual bool isCorrect() const override { return Chunk::isCorrect() && chunk->isCorrect(); }
    virtual bool isIncorrect() const override { return Chunk::isIncorrect() || chunk->isIncorrect(); }

    virtual bool isProperlyRepresented() const override { return Chunk::isProperlyRepresented() && chunk->isProperlyRepresented(); }
    virtual bool isImproperlyRepresented() const override { return Chunk::isImproperlyRepresented() || chunk->isImproperlyRepresented(); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_ENCRYPTED; }
    virtual b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= b(0)); return length; }

    virtual bool containsSameData(const Chunk& other) const override;

    virtual std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

