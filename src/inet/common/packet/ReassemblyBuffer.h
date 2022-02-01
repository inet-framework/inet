//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REASSEMBLYBUFFER_H
#define __INET_REASSEMBLYBUFFER_H

#include "inet/common/packet/ChunkBuffer.h"

namespace inet {

/**
 * This class provides functionality for reassembling out of order data chunks
 * for protocols supporting fragmentation. The reassembling algorithm requires
 * the expected length of the non-fragmented data chunk. It assumes that the
 * non-fragmented data chunk starts at 0 offset. If all data becomes available
 * up to the expected length, then the defragmentation is considered complete.
 */
class INET_API ReassemblyBuffer : public ChunkBuffer
{
  protected:
    /**
     * The total length of the reassembled data chunk.
     */
    b expectedLength;

  public:
    ReassemblyBuffer(b expectedLength = b(-1)) : expectedLength(expectedLength) {}
    ReassemblyBuffer(const ReassemblyBuffer& other) : ChunkBuffer(other), expectedLength(other.expectedLength) {}

    /**
     * Returns the expected data length.
     */
    b getExpectedLength() const { return expectedLength; }

    /**
     * Changes the expected data length.
     */
    void setExpectedLength(b expectedLength) { this->expectedLength = expectedLength; }

    /**
     * Returns true if all data is present for reassembling the data chunk.
     */
    bool isComplete() const {
        return regions.size() == 1 && regions[0].offset == b(0) && regions[0].data->getChunkLength() == expectedLength;
    }

    /**
     * Returns the reassembled data chunk if all data is present in the buffer,
     * otherwise throws an exception.
     */
    const Ptr<const Chunk> getReassembledData() const {
        if (!isComplete())
            throw cRuntimeError("Reassembly is incomplete");
        else
            return regions[0].data;
    }
};

} // namespace

#endif

