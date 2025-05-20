//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "FlowController.h"

namespace inet {
namespace quic {


ConnectionFlowController::ConnectionFlowController(uint64_t kDefaultConnectionWindowSize, Statistics *stats) : FlowController(kDefaultConnectionWindowSize, stats) { }

ConnectionFlowController::~ConnectionFlowController() { }

QuicFrame *ConnectionFlowController::generateDataBlockFrame()
{
    QuicFrame *frame = new QuicFrame();
    Ptr<DataBlockedFrameHeader> header = makeShared<DataBlockedFrameHeader>();
    header->setDataLimit(highestSendOffset);
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
