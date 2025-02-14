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

#ifndef INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLRESPONDER_H_
#define INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLRESPONDER_H_

#include "../packet/QuicFrame.h"
#include "inet/common/packet/ChunkQueue.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class Connection;

class FlowControlResponder {
public:
    FlowControlResponder(Statistics *stats);
    virtual ~FlowControlResponder();

    virtual uint64_t getRcvwnd();
    virtual void updateConsumedData(uint64_t dataSize);
    virtual bool isSendMaxDataFrame();

protected:

    virtual void updateHighestRecievedOffset(uint64_t offset) = 0;
    virtual void onDataBlockedFrameReceived(uint64_t dataLimit) = 0;
    virtual QuicFrame *generateMaxDataFrame() = 0;
    virtual QuicFrame *onMaxDataFrameLost() = 0;

    uint64_t maxReceiveOffset = 0;
    uint64_t highestRecievedOffset = 0;
    uint64_t consumedData = 0;
    uint64_t lastConsumedData = 0;
    uint64_t maxRcvwnd = 0;
    uint64_t streamId = 0;
    uint64_t lastMaxRcvOffset = 0;
    uint64_t maxDataFrameThreshold;

    //Statistic
    uint64_t rcvBlockFrameCount = 0;
    uint64_t generatedMaxDataFrameCount = 0;
    uint64_t retransmitFCUpdateCount = 0;

    Statistics *stats;
    simsignal_t rcvBlockFrameCountStat;
    simsignal_t genMaxDataFrameCountStat;
    simsignal_t maxDataFrameOffsetStat;
    simsignal_t consumedDataStat;
    simsignal_t retransmitFCUpdateStat;

    bool roundConsumedDataValue = false;

};

class ConnectionFlowControlResponder: public FlowControlResponder {
public:
    ConnectionFlowControlResponder(Connection *connection, uint64_t kDefaultConnectionWindowSize, uint64_t maxDataFrameThreshold, bool roundConsumedDataValue, Statistics *stats);
    ~ConnectionFlowControlResponder();

    virtual void updateHighestRecievedOffset(uint64_t offset);
    virtual void onDataBlockedFrameReceived(uint64_t dataLimit);
    virtual QuicFrame *generateMaxDataFrame();
    virtual QuicFrame *onMaxDataFrameLost();

private:
    Connection *connection;
};

class StreamFlowControlResponder: public FlowControlResponder {
public:
    StreamFlowControlResponder(Stream *stream, uint64_t kDefaultStreamWindowSize, uint64_t maxDataFrameThreshold, bool roundConsumedDataValue, Statistics *stats);
    ~StreamFlowControlResponder();

    virtual void updateHighestRecievedOffset(uint64_t offset);
    virtual void onDataBlockedFrameReceived(uint64_t dataLimit);
    virtual QuicFrame *generateMaxDataFrame();
    virtual QuicFrame *onMaxDataFrameLost();

private:
    Stream *stream;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_FLOWCONTROLLER_FLOWCONTROLRESPONDER_H_ */
