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

#ifndef __INET_REORDERBUFFER_H_
#define __INET_REORDERBUFFER_H_

#include "inet/common/packet/RandomAccessBuffer.h"

namespace inet {

/**
 * This class provides functionality for reordering out of order data chunks
 * for reliable connection oriented protocols. The reordering algorithm takes
 * the expected offset of the next data chunk measured in bytes. It provides
 * reordered data as a continuous data chunk at the expected offset as soon
 * as it becomes available.
 */
class INET_API ReorderBuffer : public RandomAccessBuffer
{
  protected:
    /**
     * The offset of the next expected data chunk.
     */
    int64_t expectedOffset;

  public:
    ReorderBuffer(int64_t expectedOffset = -1) : expectedOffset(expectedOffset) { }
    ReorderBuffer(const ReorderBuffer& other) : RandomAccessBuffer(other), expectedOffset(other.expectedOffset) { }

    int64_t getExpectedOffset() const { return expectedOffset; }
    void setExpectedOffset(int64_t expectedOffset) { this->expectedOffset = expectedOffset; }

    /**
     * Returns the largest next available data chunk starting at the expected
     * offset. The returned data chunk is automatically removed from the buffer.
     * If there's no available data at the expected offset, then it returns a
     * nullptr and the buffer is not modified.
     */
    std::shared_ptr<Chunk> popData() {
        if (regions.size() > 0 && regions[0].offset == expectedOffset) {
            const auto& data = regions[0].data;
            int64_t length = data->getChunkLength();
            clear(expectedOffset, length);
            expectedOffset += length;
            return data;
        }
        else
            return nullptr;
    }
};

} // namespace

#endif // #ifndef __INET_REORDERBUFFER_H_

