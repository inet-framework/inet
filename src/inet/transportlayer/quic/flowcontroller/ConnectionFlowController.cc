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


ConnectionFlowController::ConnectionFlowController(uint64_t kDefaultConnectionWindowSize, Statistics *stats) : FlowController(stats) {
    this->maxDataOffset = kDefaultConnectionWindowSize;
}

ConnectionFlowController::~ConnectionFlowController(){

}

QuicFrame *ConnectionFlowController::generateDataBlockFrame(){

    QuicFrame *frame = new QuicFrame();
    Ptr<DataBlockedFrameHeader> header = makeShared<DataBlockedFrameHeader>();
    header->setDataLimit(highestSendOffset);
    header->calcChunkLength();
    frame->setHeader(header);

    lastDataLimit = highestSendOffset;

    stats->getMod()->emit(genBlockFrameCountStat, (unsigned long)generatedBlockFrameCount++);
    stats->getMod()->emit(blockFrameOffsetStat, (unsigned long)highestSendOffset);

    return frame;
}

} /* namespace quic */
} /* namespace inet */
