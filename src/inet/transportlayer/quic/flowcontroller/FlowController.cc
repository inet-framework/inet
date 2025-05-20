//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "FlowController.h"

namespace inet {
namespace quic {

FlowController::FlowController(uint64_t maxDataOffset, Statistics *stats) : maxDataOffset(maxDataOffset), stats(stats)
{
    this->availableRwndStat = stats->createStatisticEntry("availableRwnd");
    this->genBlockFrameCountStat = stats->createStatisticEntry("generatedBlockFrameCount");
    this->rcvMaxFrameCountStat = stats->createStatisticEntry("rcvMaxFrameCount");
    this->blockFrameOffsetStat = stats->createStatisticEntry("blockFrameOffset");

    stats->getMod()->emit(rcvMaxFrameCountStat, rcvMaxFrameCount);
    stats->getMod()->emit(genBlockFrameCountStat, generatedBlockFrameCount);
}

FlowController::~FlowController() { }

uint64_t FlowController::getAvailableRwnd()
{
    return (maxDataOffset - highestSendOffset);
}

void FlowController::onMaxFrameReceived(uint64_t newMaxDataOffset)
{
    EV_DEBUG << "received MaxStreamFrame or MaxFrame. New Max_receive_offset = " << newMaxDataOffset << endl;

    if(maxDataOffset < newMaxDataOffset){
        maxDataOffset = newMaxDataOffset;
        rcvMaxFrameCount++;
        stats->getMod()->emit(rcvMaxFrameCountStat, rcvMaxFrameCount);
    }

    stats->getMod()->emit(availableRwndStat, getAvailableRwnd());
}

void FlowController::onStreamFrameSent(uint64_t size)
{
    highestSendOffset += size;
    stats->getMod()->emit(availableRwndStat, getAvailableRwnd());
}

bool FlowController::isDataBlockedFrameWasSend()
{
    return ((lastDataLimit == highestSendOffset) ? true : false);
}

void FlowController::onStreamFrameLost(uint64_t size)
{
    highestSendOffset -= size;
}

void FlowController::setMaxDataOffset(uint64_t newMaxDataOffset)
{
    if (newMaxDataOffset > maxDataOffset) {
        maxDataOffset = newMaxDataOffset;
    }
}

} /* namespace quic */
} /* namespace inet */
