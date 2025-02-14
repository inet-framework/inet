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

#include "FlowController.h"

namespace inet {
namespace quic {

FlowController::FlowController(Statistics *stats) {

    this->stats = stats;
    this->availableRwndStat = stats->createStatisticEntry("availableRwnd");
    this->genBlockFrameCountStat = stats->createStatisticEntry("generatedBlockFrameCount");
    this->rcvMaxFrameCountStat = stats->createStatisticEntry("rcvMaxFrameCount");
    this->blockFrameOffsetStat = stats->createStatisticEntry("blockFrameOffset");

    stats->getMod()->emit(rcvMaxFrameCountStat, (unsigned long)rcvMaxFrameCount);
    stats->getMod()->emit(genBlockFrameCountStat, (unsigned long)generatedBlockFrameCount);
}

FlowController::~FlowController() {
}

uint64_t FlowController::getAvailableRwnd(){
    return (maxDataOffset - highestSendOffset);
}

void FlowController::onMaxFrameReceived(uint64_t newMaxDataOffset){
    EV_DEBUG << "received MaxStreamFrame or MaxFrame. New Max_receive_offset = " << newMaxDataOffset << endl;

    if(maxDataOffset < newMaxDataOffset){
         maxDataOffset = newMaxDataOffset;
         rcvMaxFrameCount++;
         stats->getMod()->emit(rcvMaxFrameCountStat, (unsigned long)rcvMaxFrameCount);
     }

    stats->getMod()->emit(availableRwndStat, (unsigned long)getAvailableRwnd());
}

void FlowController::onStreamFrameSent(uint64_t size){
    highestSendOffset += size;
    stats->getMod()->emit(availableRwndStat, (unsigned long)getAvailableRwnd());
}

bool FlowController::isDataBlockedFrameWasSend(){
    return ((lastDataLimit == highestSendOffset) ? true : false);
}

void FlowController::onStreamFrameLost(uint64_t size){
    highestSendOffset -= size;
}

} /* namespace quic */
} /* namespace inet */
