//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_REASSEMBLYBUFFER_H_
#define __INET_REASSEMBLYBUFFER_H_

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
    ReassemblyBuffer(b expectedLength = b(-1)) : expectedLength(expectedLength) { }
    ReassemblyBuffer(const ReassemblyBuffer& other) : ChunkBuffer(other), expectedLength(other.expectedLength) { }

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

#endif // #ifndef __INET_REASSEMBLYBUFFER_H_

