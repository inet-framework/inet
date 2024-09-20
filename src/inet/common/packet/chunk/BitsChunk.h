//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BITSCHUNK_H
#define __INET_BITSCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a sequence of bits. This can be useful
 * when the actual data is important because. For example, when an external
 * program sends or receives the data, or in hardware in the loop simulations.
 */
class INET_API BitsChunk : public Chunk
{
    friend class Chunk;

  protected:
    /**
     * The data bits as is.
     */
    std::vector<bool> bits;

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
    BitsChunk();
    BitsChunk(const BitsChunk& other) = default;
    BitsChunk(const std::vector<bool>& bits);

    BitsChunk *dup() const override { return new BitsChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<BitsChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<bool>& getBits() const { return bits; }
    void setBits(const std::vector<bool>& bits);

    size_t getBitArraySize() const { return bits.size(); }
    bool getBit(int index) const { return bits.at(index); }
    void setBit(int index, bool bit);
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_BITS; }
    b getChunkLength() const override { return b(bits.size()); }

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

