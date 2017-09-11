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

#include "inet/common/packet/ReorderBuffer.h"

namespace inet {

b ReorderBuffer::getAvailableDataLength() const
{
    if (regions.size() > 0 && regions[0].offset == expectedOffset)
        return regions[0].data->getChunkLength();
    else
        return b(0);
}

const Ptr<const Chunk> ReorderBuffer::popAvailableData(b length)
{
    if (regions.size() > 0 && regions[0].offset == expectedOffset) {
        auto data = regions[0].data;
        b dataLength = data->getChunkLength();
        if (length != b(-1)) {
            data = data->peek(Chunk::ForwardIterator(b(0)), length);
            dataLength = length;
        }
        clear(expectedOffset, dataLength);
        expectedOffset += dataLength;
        return data;
    }
    else
        return nullptr;
}

} // namespace inet

