//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
