//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
