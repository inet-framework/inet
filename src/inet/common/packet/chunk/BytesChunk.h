//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BYTESCHUNK_H
#define __INET_BYTESCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a sequence of bytes. This can be useful
 * when the actual data is important because. For example, when an external
 * program sends or receives the data, or in hardware in the loop simulations.
 */
class INET_API BytesChunk : public Chunk
{
    friend class Chunk;

  protected:
    /**
     * The data bytes as is.
     */
    std::vector<uint8_t> bytes;

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
    BytesChunk();
    BytesChunk(const BytesChunk& other) = default;
    BytesChunk(const std::vector<uint8_t>& bytes);
    BytesChunk(const uint8_t *buffer, size_t bufLen) : Chunk(), bytes(buffer, buffer + bufLen) {}

    BytesChunk *dup() const override { return new BytesChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<BytesChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Field accessor functions */
    //@{
    const std::vector<uint8_t>& getBytes() const { return bytes; }
    void setBytes(const std::vector<uint8_t>& bytes);

    size_t getByteArraySize() const { return bytes.size(); }
    uint8_t getByte(int index) const { return bytes.at(index); }
    void setByte(int index, uint8_t byte);
    //@}

    /** @name Utility functions */
    //@{
    size_t copyToBuffer(uint8_t *buffer, size_t bufferLength) const;
    void copyFromBuffer(const uint8_t *buffer, size_t bufferLength);
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_BYTES; }
    b getChunkLength() const override { return B(bytes.size()); }

    bool containsSameData(const Chunk& other) const override;

    bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override;
    bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override;
    bool canInsertAt(const Ptr<const Chunk>& chunk, b offset) const override;

    bool canRemoveAtFront(b length) const override { return b(length).get() % 8 == 0; }
    bool canRemoveAtBack(b length) const override { return b(length).get() % 8 == 0; }
    bool canRemoveAt(b offset, b length) const override { return b(offset).get() % 8 == 0 && b(length).get() % 8 == 0; }

    std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

