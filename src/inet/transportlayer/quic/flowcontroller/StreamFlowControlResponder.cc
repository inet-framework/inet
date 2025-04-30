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


