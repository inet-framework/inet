//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "FlowControlResponder.h"

namespace inet {
namespace quic {

FlowControlResponder::FlowControlResponder(uint64_t initialWindowSize, uint64_t maxDataFrameThreshold, bool roundConsumedDataValue, Statistics *stats) : maxReceiveOffset(initialWindowSize), maxRcvwnd(initialWindowSize), maxDataFrameThreshold(maxDataFrameThreshold), stats(stats), roundConsumedDataValue(roundConsumedDataValue)
{
    this->rcvBlockFrameCountStat = stats->createStatisticEntry("rcvBlockFrameCount");
    this->genMaxDataFrameCountStat = stats->createStatisticEntry("generatedMaxDataFrameCount");
    this->maxDataFrameOffsetStat = stats->createStatisticEntry("maxDataFrameOffset");
    this->consumedDataStat = stats->createStatisticEntry("consumedData");
    this->maxDataFrameLostCountStat = stats->createStatisticEntry("maxDataFrameLostCount");

    stats->getMod()->emit(rcvBlockFrameCountStat, rcvBlockFrameCount);
    stats->getMod()->emit(genMaxDataFrameCountStat, generatedMaxDataFrameCount);
    stats->getMod()->emit(maxDataFrameLostCountStat, maxDataFrameLostCount);
}

FlowControlResponder::~FlowControlResponder() { }

void FlowControlResponder::updateConsumedData(uint64_t dataSize)
{
    lastConsumedData = consumedData;
    consumedData += dataSize;
    stats->getMod()->emit(consumedDataStat, consumedData);
}

uint64_t FlowControlResponder::getRcvwnd()
{
    return (maxReceiveOffset - highestRecievedOffset);
}

bool FlowControlResponder::isSendMaxDataFrame()
{
    // In Doc "Flow Control in QUIC" maxDataFrameThreshold = maxRcvwnd/2
    EV_DEBUG << "isSendMaxDataFrame: " << maxReceiveOffset << " - " << consumedData << " <= " << maxDataFrameThreshold << " = " << ((maxReceiveOffset - consumedData) <= (maxDataFrameThreshold)) << endl;
    return ((((maxReceiveOffset - consumedData) <= (maxDataFrameThreshold)) && (!(lastMaxRcvOffset == maxReceiveOffset))) ? true : false);
}

} /* namespace quic */
} /* namespace inet */
