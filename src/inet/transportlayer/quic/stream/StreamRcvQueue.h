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

#ifndef INET_APPLICATIONS_QUIC_STREAMRCVQUEUE_H_
#define INET_APPLICATIONS_QUIC_STREAMRCVQUEUE_H_

#include "Stream.h"
#include "inet/common/packet/ReorderBuffer.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class Stream;

class StreamRcvQueue {
public:
    StreamRcvQueue(Stream *stream, Statistics *stats);
    virtual ~StreamRcvQueue();

    ReorderBuffer reorderBuffer;

protected:
    Stream *stream = nullptr;    // the stream that owns this queue
    uint64_t startSeq = 0;

    //Statistic
    Statistics *stats;
    simsignal_t streamRcvFrameStartOffsetStat;
    simsignal_t streamRcvFrameEndOffsetStat;

public:
    bool hasDataForApp();
    void push(Ptr<const Chunk> data, uint64_t offset);
    const Ptr<const Chunk>  pop(B dataSize);
    uint64_t offsetToSeq(B offs) const { return (uint64_t)offs.get(); }

private:
    uint64_t numStreamFramesReceived = 0;
    uint64_t numDuplFrames = 0;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_STREAMRCVQUEUE_H_ */
