//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

BlockAckRecord::BlockAckRecord(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize) :
    originatorAddress(originatorAddress),
    tid(tid),
    startingSequenceNumber(startingSequenceNumber),
    windowSize(std::min(64, bufferSize))
{
    if (bufferSize < 1 || bufferSize > 64)
        throw cRuntimeError("Unsupported BlockAck receive buffer size: %d", bufferSize);
}

void BlockAckRecord::blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    if (header->getAckPolicy() == BLOCK_ACK)
        dataFrameReceived(header);
}

void BlockAckRecord::dataFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    if (header->getTransmitterAddress() != originatorAddress || header->getTid() != tid ||
            header->getType() != ST_DATA_WITH_QOS ||
            header->getFragmentNumber() != 0 || header->getMoreFragments() || header->isIncorrect())
        return;
    // IEEE Std 802.11-2024, 10.25.6.3 b): each related Data frame updates the record; ignore the old half-space.
    auto sequenceNumber = header->getSequenceNumber();
    if (sequenceNumber != startingSequenceNumber && !(startingSequenceNumber < sequenceNumber))
        return;
    if (!(sequenceNumber < startingSequenceNumber + windowSize))
        blockAckReqReceived(sequenceNumber - windowSize + 1);
    acknowledgmentState[SequenceControlField(sequenceNumber.get(), 0)] = true;
}

bool BlockAckRecord::getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber)
{
    return fragmentNumber == 0 &&
            (sequenceNumber == startingSequenceNumber || startingSequenceNumber < sequenceNumber) &&
            sequenceNumber < startingSequenceNumber + windowSize &&
            containsKey(acknowledgmentState, SequenceControlField(sequenceNumber.get(), fragmentNumber));
}

void BlockAckRecord::blockAckReqReceived(SequenceNumberCyclic sequenceNumber)
{
    // IEEE Std 802.11-2024, 10.25.6.3 c): retain the overlap; clear new positions.
    if (!(startingSequenceNumber < sequenceNumber))
        return;
    startingSequenceNumber = sequenceNumber;
    for (auto it = acknowledgmentState.begin(); it != acknowledgmentState.end(); ) {
        auto position = SequenceNumberCyclic(it->first.getSequenceNumber());
        if ((position != startingSequenceNumber && !(startingSequenceNumber < position)) ||
                !(position < startingSequenceNumber + windowSize))
            it = acknowledgmentState.erase(it);
        else
            ++it;
    }
}

void BlockAckRecord::removeAckStates(SequenceNumberCyclic sequenceNumber)
{
    // Kept for older callers. Receive-window movement owns status retirement.
    blockAckReqReceived(sequenceNumber);
}

} /* namespace ieee80211 */
} /* namespace inet */
