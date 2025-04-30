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


StreamFlowController::StreamFlowController(uint64_t streamId, uint64_t kDefaultStreamWindowSize, Statistics *stats) : FlowController(stats) {
    this->streamId = streamId;
    this->maxDataOffset = kDefaultStreamWindowSize;
}

StreamFlowController::~StreamFlowController(){

}

QuicFrame *StreamFlowController::generateDataBlockFrame(){

    QuicFrame *frame = new QuicFrame();
    Ptr<StreamDataBlockedFrameHeader> header = makeShared<StreamDataBlockedFrameHeader>();
    header->setStreamId(streamId);
    header->setStreamDataLimit(highestSendOffset);
    header->calcChunkLength();
    frame->setHeader(header);

    lastDataLimit = highestSendOffset;

    generatedBlockFrameCount++;
    stats->getMod()->emit(genBlockFrameCountStat, generatedBlockFrameCount);
    stats->getMod()->emit(blockFrameOffsetStat, highestSendOffset);

    return frame;
}

} /* namespace quic */
} /* namespace inet */

