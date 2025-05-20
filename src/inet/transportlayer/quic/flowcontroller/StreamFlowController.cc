//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "FlowController.h"

namespace inet {
namespace quic {


StreamFlowController::StreamFlowController(uint64_t streamId, uint64_t kDefaultStreamWindowSize, Statistics *stats) : FlowController(kDefaultStreamWindowSize, stats), streamId(streamId) { }

StreamFlowController::~StreamFlowController() { }

QuicFrame *StreamFlowController::generateDataBlockFrame()
{
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

