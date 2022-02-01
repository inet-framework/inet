//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REORDERBUFFER_H
#define __INET_REORDERBUFFER_H

#include "inet/common/packet/ChunkBuffer.h"

namespace inet {

/**
 * This class provides functionality for reordering out of order data chunks
 * for reliable connection oriented protocols. The reordering algorithm takes
 * the expected offset of the next data chunk. It provides reordered data as a
 * continuous data chunk at the expected offset as soon as it becomes available.
 */
class INET_API ReorderBuffer : public ChunkBuffer
{
  protected:
    /**
     * The offset of the next expected data chunk.
     */
    b expectedOffset;

  public:
    ReorderBuffer(b expectedOffset = b(-1)) : expectedOffset(expectedOffset) {}
    ReorderBuffer(const ReorderBuffer& other) : ChunkBuffer(other), expectedOffset(other.expectedOffset) {}

    /**
     * Returns the offset of the next expected data chunk.
     */
    b getExpectedOffset() const { return expectedOffset; }

    /**
     * Changes the offset of the next expected data chunk.
     */
    void setExpectedOffset(b expectedOffset) { this->expectedOffset = expectedOffset; }

    /**
     * Returns the length of the largest next available data chunk starting at
     * the expected offset.
     */
    b getAvailableDataLength() const;

    /**
     * Returns the largest next available data chunk starting at the expected
     * offset. The returned data chunk is automatically removed from the buffer.
     * If there's no available data at the expected offset, then it returns a
     * nullptr and the buffer is not modified.
     */
    const Ptr<const Chunk> popAvailableData(b length = b(-1));
};

} // namespace

#endif

