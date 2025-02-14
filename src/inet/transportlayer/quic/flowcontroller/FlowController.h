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

#ifndef INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_
#define INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_

#include "../packet/QuicFrame.h"
#include "inet/common/packet/ChunkQueue.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class FlowController {
public:
    FlowController(Statistics *stats);
    virtual ~FlowController();

    virtual uint64_t getAvailableRwnd();
    virtual void onMaxFrameReceived(uint64_t maxDataOffset);
    virtual void onStreamFrameSent(uint64_t size);
    virtual bool isDataBlockedFrameWasSend(); //check if DataBlockedFrame was already send for current highestSendOffset
    virtual void onStreamFrameLost(uint64_t size); //take into account size of retransmitted stream frames (the implementation has only the sendQueue and no retransmission Queue)

protected:
    virtual QuicFrame *generateDataBlockFrame() = 0;

protected:
    uint64_t maxDataOffset = 0;
    uint64_t highestSendOffset = 0;
    uint64_t streamId = 0;
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
    ConnectionFlowController(uint64_t kDefaultConnectionWindowSize, Statistics *stats);
    ~ConnectionFlowController();

    virtual QuicFrame *generateDataBlockFrame();
};

class StreamFlowController: public FlowController {
public:
    StreamFlowController(uint64_t streamId, uint64_t kDefaultStreamWindowSize, Statistics *stats);
    ~StreamFlowController();

    virtual QuicFrame *generateDataBlockFrame();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLLER_H_ */
