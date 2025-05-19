//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "../packet/QuicMaxStreamDataFrame.h"
#include "FlowControlResponder.h"

namespace inet {
namespace quic {

StreamFlowControlResponder::StreamFlowControlResponder(Stream *stream, uint64_t kDefaultStreamWindowSize, uint64_t maxDataFrameThreshold, bool roundConsumedDataValue, Statistics *stats):FlowControlResponder(stats){

    this->stream = stream;
    this->streamId = stream->id;
    this->maxRcvwnd = kDefaultStreamWindowSize;
    this->maxReceiveOffset = maxRcvwnd;
    this->maxDataFrameThreshold = maxDataFrameThreshold;
    this->roundConsumedDataValue = roundConsumedDataValue;
}

StreamFlowControlResponder::~StreamFlowControlResponder(){
}

void StreamFlowControlResponder::updateHighestRecievedOffset(uint64_t offset){
    if(highestRecievedOffset < offset){
        highestRecievedOffset = offset;
    }
}

QuicFrame *StreamFlowControlResponder::generateMaxDataFrame(){

    generatedMaxDataFrameCount++;
    lastMaxRcvOffset = maxReceiveOffset;

    EV_DEBUG << "StreamFlowControlResponder::generateMaxDataFrame: increase maxReceiveOffset from " << maxReceiveOffset;

    if(!roundConsumedDataValue){
        maxReceiveOffset = consumedData + maxRcvwnd;
    } else {
        if((consumedData - lastConsumedData) < maxDataFrameThreshold){
            maxReceiveOffset += maxRcvwnd - maxDataFrameThreshold;
        }else{
            maxReceiveOffset = consumedData + maxRcvwnd;
        }
    }

    EV_DEBUG << " to " << maxReceiveOffset << endl;
    auto *frame = new QuicMaxStreamDataFrame(stream, maxReceiveOffset);

    stats->getMod()->emit(genMaxDataFrameCountStat, generatedMaxDataFrameCount);
    stats->getMod()->emit(maxDataFrameOffsetStat, maxReceiveOffset);

    return frame;
}

QuicFrame *StreamFlowControlResponder::onMaxDataFrameLost(){
    EV_DEBUG << "retransmit FC update" << endl;
    maxDataFrameLostCount++;
    stats->getMod()->emit(maxDataFrameLostCountStat, maxDataFrameLostCount);


    if(isSendMaxDataFrame()){
        return generateMaxDataFrame();
    } else {
        auto *frame = new QuicMaxStreamDataFrame(stream, maxReceiveOffset);
        return frame;
    }
}


void StreamFlowControlResponder::onDataBlockedFrameReceived(uint64_t dataLimit){
    EV_DEBUG << "received Data_Blocked frame. Blocking offset= " << dataLimit << endl;
    rcvBlockFrameCount++;
    stats->getMod()->emit(rcvBlockFrameCountStat, rcvBlockFrameCount);
}

} /* namespace quic */
} /* namespace inet */


