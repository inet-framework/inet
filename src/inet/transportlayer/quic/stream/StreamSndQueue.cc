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

#include "StreamSndQueue.h"

namespace inet {
namespace quic {

void StreamSndQueue::addData(const Ptr<const Chunk> data) {
    sendBuffer.replace(addOffset, data);
    addOffset += data->getChunkLength();
}

void StreamSndQueue::addOutstandingRegion(Region region)
{
    std::vector<Region>::iterator it = outstandingRegions.begin();
    for (; it<outstandingRegions.end() && it->offset < region.offset; it++);
    outstandingRegions.insert(it, region);
}

bool StreamSndQueue::removeOutstandingRegion(b offset, b length)
{
    for (std::vector<Region>::iterator it = outstandingRegions.begin(); it<outstandingRegions.end(); it++) {
        if (it->offset == offset && it->length == length) {
            outstandingRegions.erase(it);
            return true;
        }
    }
    return false;
}

void StreamSndQueue::dataLost(b offset, b length) {
    removeOutstandingRegion(offset, length);
}

void StreamSndQueue::dataAcked(b offset, b length) {
    if (removeOutstandingRegion(offset, length)) {
        sendBuffer.clear(offset, length);
    }
}

bool StreamSndQueue::findData(Region &region, int &bufferIndex, bool adjustBounds)
{
    ASSERT(region.offset >= b(0) && region.length >= b(0) && bufferIndex >= 0);
    for (; bufferIndex<sendBuffer.getNumRegions(); bufferIndex++) {
        if (region.offset < sendBuffer.getRegionEndOffset(bufferIndex)) {
            // gesuchte Region im Buffer befindet sich am Index i

            if (region.offset < sendBuffer.getRegionStartOffset(bufferIndex)) {
                // wanted offset is before the offset of the region in the buffer
                if (!adjustBounds) {
                    return false;
                }

                if (region.length != b(0) && (region.offset + region.length) <= sendBuffer.getRegionStartOffset(bufferIndex)) {
                    // wanted region does not interleave with the region in the buffer,
                    // so there are no data to send in the wanted region
                    break;
                }
                // wanted region interleaves with the region in the buffer, adjust offset and length accordingly
                if (region.length != b(0)) {
                    region.length -= sendBuffer.getRegionStartOffset(bufferIndex) - region.offset;
                }
                region.offset = sendBuffer.getRegionStartOffset(bufferIndex);
            }

            b remainingLength = sendBuffer.getRegionEndOffset(bufferIndex) - region.offset;
            if (region.length == b(0) || remainingLength < region.length) {
                // the remaining length of the data in the buffer from the wanted offset is smaller than the wanted length
                if (!adjustBounds) {
                    return false;
                }
                // adjust length
                region.length = remainingLength;
            }
            return true;
        }
    }
    return false;
}

StreamSndQueue::Region StreamSndQueue::getSendRegion(b offset, b length)
{
    if (sendBuffer.getNumRegions() == 0 || offset < b(0) || length < b(0)) {
        // Buffer is empty, there are no data to send
        // or offset or length have a wrong value
        return Region(b(-1), b(0));
    }
    Region sendRegion = Region(offset, length);
    // Looking for data in the send region
    int bufferIndex = 0;
    if (findData(sendRegion, bufferIndex, true)) {
        // data found
        return sendRegion;
    }
    // no data found in the given region
    return Region(b(-1), b(0));
}

StreamSndQueue::Region StreamSndQueue::getNextSendRegion(b startOffset)
{
    if (sendBuffer.getNumRegions() == 0) {
        // buffer is empty, there are no data to send.
        return Region(b(-1), b(0));
    }
    Region sendRegion = Region(startOffset, b(0));
    int bufferIndex = 0;
    for (std::vector<Region>::iterator it = outstandingRegions.begin(); it<outstandingRegions.end(); it++) {
        if (sendRegion.offset < it->offset) {
            // There is a gap in the outstaindingRegions.
            sendRegion.length = it->offset - sendRegion.offset;
            if (findData(sendRegion, bufferIndex, true)) {
                // There are data in the gap, return this region.
                return sendRegion;
            }
            if (bufferIndex == sendBuffer.getNumRegions()) {
                // We reached the end of the data buffer while searching. There are no more data to send.
                return Region(b(-1), b(0));
            }
        }
        // Forward offset
        if (sendRegion.offset < (it->offset + it->length)) {
            sendRegion.offset = (it->offset + it->length);
        }
    }
    // There are no gaps in the outstandingRegions where we could find data to send
    // Check if there are data to send at the end of the last outstandingRegion
    sendRegion.length = b(0);
    if (findData(sendRegion, bufferIndex, true)) {
        // There are data to send tt the end of the last oustandingRegion, return this region.
        return sendRegion;
    }
    // Even at the end of the last outstandingRegion are not data to send.
    return Region(b(-1), b(0));
}

const Ptr<const Chunk> StreamSndQueue::getData(b offset, b length)
{
    Region region = Region(offset, length);
    int bufferIndex = 0;
    if (findData(region, bufferIndex, false)) {
        auto data = sendBuffer.getRegionData(bufferIndex);
        ASSERT(region.offset >= sendBuffer.getRegionStartOffset(bufferIndex));
        ASSERT(region.length <= (sendBuffer.getRegionLength(bufferIndex) - (region.offset - sendBuffer.getRegionStartOffset(bufferIndex))));
        if (region.offset > sendBuffer.getRegionStartOffset(bufferIndex)
                || region.length < sendBuffer.getRegionLength(bufferIndex)) {
            data = data->peek(Chunk::ForwardIterator(region.offset - sendBuffer.getRegionStartOffset(bufferIndex)), region.length);
        }
        return data;
    }
    return nullptr;
}

uint64_t StreamSndQueue::getNextDataLengthToSend()
{
    Region sendRegion = getNextSendRegion(b(0));
    return B(sendRegion.length).get();
}

uint64_t StreamSndQueue::getTotalDataLengthToSend()
{
    uint64_t totalLength = 0;
    b offset = b(0);
    Region sendRegion = Region(b(-1), b(0));
    while ( (sendRegion = getNextSendRegion(offset), sendRegion.offset != b(-1)) ) {
        totalLength += B(sendRegion.length).get();
        offset = sendRegion.offset + sendRegion.length;
    }
    return totalLength;
}

uint64_t StreamSndQueue::getTotalDataLength()
{
    uint64_t totalLength = 0;
    for (int i=0; i<sendBuffer.getNumRegions(); i++) {
        totalLength += B(sendBuffer.getRegionLength(i)).get();
    }
    return totalLength;
}

} /* namespace quic */
} /* namespace inet */
