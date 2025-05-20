//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_

#include "../packet/QuicFrame.h"
#include "inet/common/packet/ChunkQueue.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class FlowController {
public:
    FlowController(uint64_t maxDataOffset, Statistics *stats);
    virtual ~FlowController();

    virtual uint64_t getAvailableRwnd();
    virtual void onMaxFrameReceived(uint64_t maxDataOffset);
    virtual void onStreamFrameSent(uint64_t size);
    virtual bool isDataBlockedFrameWasSend(); //check if DataBlockedFrame was already send for current highestSendOffset
    virtual void onStreamFrameLost(uint64_t size); //take into account size of retransmitted stream frames (the implementation has only the sendQueue and no retransmission Queue)
    virtual void setMaxDataOffset(uint64_t maxDataOffset);

protected:
    virtual QuicFrame *generateDataBlockFrame() = 0;

protected:
    uint64_t maxDataOffset = 0;
    uint64_t highestSendOffset = 0;
    uint64_t lastDataLimit = 0;

    // Statistic
    uint64_t generatedBlockFrameCount = 0;
    uint64_t rcvMaxFrameCount = 0;

    Statistics *stats;
    simsignal_t availableRwndStat;
    simsignal_t genBlockFrameCountStat;
    simsignal_t rcvMaxFrameCountStat;
    simsignal_t blockFrameOffsetStat;
};

class ConnectionFlowController: public FlowController {
public:
    ConnectionFlowController(uint64_t kDefaultStreamWindowSize, Statistics *stats);
    ~ConnectionFlowController();

    virtual QuicFrame *generateDataBlockFrame();
};

class StreamFlowController: public FlowController {
public:
    StreamFlowController(uint64_t streamId, uint64_t kDefaultStreamWindowSize, Statistics *stats);
    ~StreamFlowController();

    virtual QuicFrame *generateDataBlockFrame();

private:
    uint64_t streamId = 0;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_ */
