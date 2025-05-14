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

#ifndef INET_APPLICATIONS_QUIC_STREAM_STREAMSNDQUEUE_H_
#define INET_APPLICATIONS_QUIC_STREAM_STREAMSNDQUEUE_H_

#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {
namespace quic {

class StreamSndQueue {
public:
    class Region {
    public:
        Region(b offset, b length) {
            this->offset = offset;
            this->length = length;
        };
        b offset;
        b length;
    };

    StreamSndQueue() { };
    virtual ~StreamSndQueue() { };

    /**
     * Adds data to this queue.
     *
     * @param data The data to add.
     */
    void addData(const Ptr<const Chunk> data);

    /**
     * Stores a region as outstanding.
     *
     * @param region the Region to store as outstanding.
     */
    void addOutstandingRegion(Region region);

    /**
     * Removes the region as outstanding.
     *
     * @param offset The offset of the region.
     * @param length The length of the region.
     */
    void dataLost(b offset, b length);

    /**
     * Removes the region as outstanding and deletes the data in the send buffer.
     *
     * @param offset The offset of the region.
     * @param length The length of the region.
     */
    void dataAcked(b offset, b length);

    /**
     * Returns the data from the send buffer for the given region.
     *
     * @param offset The offset of the region.
     * @param length The length of the region.
     * @return The data from the send buffer.
     */
    const Ptr<const Chunk> getData(b offset, b length);

    Region getSendRegion(b offset, b length);

    /**
     * Finds the next region of data to send. For that it uses the available data in the send buffer
     * minus the stored outstanding regions.
     *
     * @param startOffset Gives the offset where to start the search.
     * @return the next region to send. If the returned region contains an offset of b(-1), nothing to send is available.
     */
    Region getNextSendRegion(b startOffset = b(0));

    /**
     * @return The size in bytes of data ready to send in one chunk (the first).
     * This might be less than getTotalDataLengthToSend(), when we have scattered data.
     * For example, if we store two data chunks ready to send, first chunk of size 500 at offset 1000,
     * second chunk of size 800 at offset 2000; because of the gap, we cannot send both chunks in one
     * stream frame. Thus, this function would return 500.
     */
    uint64_t getNextDataLengthToSend();

    /**
     * @return The total size in bytes of data ready to send.
     * For example, if we store two data chunks ready to send, first chunk of size 500 at offset 1000,
     * second chunk of size 800 at offset 2000. This function would return 1300.
     */
    uint64_t getTotalDataLengthToSend();

    /**
     * @return The total size of data in this queue.
     */
    uint64_t getTotalDataLength();

    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const {
        std::stringstream str;
        str << "StreamSndQueue(";
        str << "sendBuffer=" << sendBuffer.str();
        str << ", ";
        str << "outstandingRegions=[";
        for (int i=0; i<outstandingRegions.size(); i++) {
            str << "(offset=" << outstandingRegions[i].offset << ", length=" << outstandingRegions[i].length << "), ";
        }
        str << "])";
        return str.str();
    }

private:
    ChunkBuffer sendBuffer;
    b addOffset = B(0);
    std::vector<Region> outstandingRegions;

    bool removeOutstandingRegion(b offset, b length);

    /**
     * Finds data in the send buffer for the given region.
     *
     * @param region The region where to find the data in the send buffer. If adjustBounds is true, it modifies the offset and length in the given region.
     * @param bufferIndex Gives the start index for the search. It modifies bufferIndex while searching.
     * @param adjustBounds If true, offset and length in the region are adjusted according to the available data in the send buffer.
     * @return true if data is available in the send buffer, otherwise false.
     */
    bool findData(Region &region, int &bufferIndex, bool adjustBounds);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_STREAM_STREAMSNDQUEUE_H_ */
