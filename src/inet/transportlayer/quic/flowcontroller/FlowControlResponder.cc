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

#include "FlowControlResponder.h"

namespace inet {
namespace quic {

FlowControlResponder::FlowControlResponder(Statistics *stats) {
    this->stats = stats;
    this->rcvBlockFrameCountStat = stats->createStatisticEntry("rcvBlockFrameCount");
    this->genMaxDataFrameCountStat = stats->createStatisticEntry("generatedMaxDataFrameCount");
    this->maxDataFrameOffsetStat = stats->createStatisticEntry("maxDataFrameOffset");
    this->consumedDataStat = stats->createStatisticEntry("consumedData");
    this->retransmitFCUpdateStat = stats->createStatisticEntry("retransmitFCUpdateCount");

    stats->getMod()->emit(rcvBlockFrameCountStat, (unsigned long)rcvBlockFrameCount);
    stats->getMod()->emit(genMaxDataFrameCountStat, (unsigned long)generatedMaxDataFrameCount);
    stats->getMod()->emit(retransmitFCUpdateStat, (unsigned long)retransmitFCUpdateCount);
}

FlowControlResponder::~FlowControlResponder() {
    // TODO Auto-generated destructor stub
}

void FlowControlResponder::updateConsumedData(uint64_t dataSize){
    lastConsumedData = consumedData;
    consumedData += dataSize;
    stats->getMod()->emit(consumedDataStat, (unsigned long)consumedData);
}

uint64_t FlowControlResponder::getRcvwnd() {
    return (maxReceiveOffset - highestRecievedOffset);
}

bool FlowControlResponder::isSendMaxDataFrame(){
    // In Doc "Flow Control in QUIC" maxDataFrameThreshold = maxRcvwnd/2
    EV_DEBUG << "isSendMaxDataFrame: " << maxReceiveOffset << " - " << consumedData << " <= " << maxDataFrameThreshold << " = " << ((maxReceiveOffset - consumedData) <= (maxDataFrameThreshold)) << endl;
    return ((((maxReceiveOffset - consumedData) <= (maxDataFrameThreshold)) && (!(lastMaxRcvOffset == maxReceiveOffset))) ? true : false);
}

} /* namespace quic */
} /* namespace inet */
