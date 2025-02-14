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

    void addData(const Ptr<const Chunk> data);
    void addOutstandingRegion(Region region);

    void dataLost(b offset, b length);
    void dataAcked(b offset, b length);
    const Ptr<const Chunk> getData(b offset, b length);

    Region getSendRegion(b offset, b length);
    Region getNextSendRegion(b startOffset = b(0));
    uint64_t getNextDataLengthToSend();
    uint64_t getTotalDataLengthToSend();
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
    bool findData(Region &region, int &bufferIndex, bool adjustBounds);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_STREAM_STREAMSNDQUEUE_H_ */
