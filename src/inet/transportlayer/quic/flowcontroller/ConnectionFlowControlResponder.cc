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

#include "../packet/QuicMaxDataFrame.h"
#include "FlowControlResponder.h"

namespace inet {
namespace quic {

ConnectionFlowControlResponder::ConnectionFlowControlResponder(Connection *connection, uint64_t kDefaultConnectionWindowSize, uint64_t maxDataFrameThreshold, bool roundConsumedDataValue, Statistics *stats):FlowControlResponder(stats){
    this->connection = connection;
    this->maxRcvwnd = kDefaultConnectionWindowSize;
    this->maxReceiveOffset = maxRcvwnd;
    this->maxDataFrameThreshold = maxDataFrameThreshold;
    this->roundConsumedDataValue = roundConsumedDataValue;
}

ConnectionFlowControlResponder::~ConnectionFlowControlResponder(){

}

void ConnectionFlowControlResponder::updateHighestRecievedOffset(uint64_t offset){
    highestRecievedOffset += offset;
}

QuicFrame *ConnectionFlowControlResponder::generateMaxDataFrame(){

    generatedMaxDataFrameCount++;
    lastMaxRcvOffset = maxReceiveOffset;

    if(!roundConsumedDataValue){
        maxReceiveOffset = consumedData + maxRcvwnd;
    } else {
        if((consumedData - lastConsumedData) < maxDataFrameThreshold){
            maxReceiveOffset += maxRcvwnd - maxDataFrameThreshold;
        }else{
            maxReceiveOffset = consumedData + maxRcvwnd;
        }
    }

    auto *frame = new QuicMaxDataFrame(connection, maxReceiveOffset);

    stats->getMod()->emit(genMaxDataFrameCountStat, generatedMaxDataFrameCount);
    stats->getMod()->emit(maxDataFrameOffsetStat, maxReceiveOffset);

    return frame;
}

void ConnectionFlowControlResponder::onDataBlockedFrameReceived(uint64_t dataLimit){
    EV_DEBUG << "received Data_Blocked frame. Connection-limit = " << dataLimit << endl;
    rcvBlockFrameCount++;
    stats->getMod()->emit(rcvBlockFrameCountStat, rcvBlockFrameCount);
}


QuicFrame *ConnectionFlowControlResponder::onMaxDataFrameLost(){
    EV_DEBUG << "retransmit CFC update" << endl;
    maxDataFrameLostCount++;
    stats->getMod()->emit(maxDataFrameLostCountStat, maxDataFrameLostCount);

    if(isSendMaxDataFrame()){
        return generateMaxDataFrame();
    } else {
        auto *frame = new QuicMaxDataFrame(connection, maxReceiveOffset);
        return frame;
    }
}

} /* namespace quic */
} /* namespace inet */
