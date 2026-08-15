//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

BlockAckRecord::BlockAckRecord(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber) :
    originatorAddress(originatorAddress),
    tid(tid),
    startingSequenceNumber(startingSequenceNumber)
{
}

void BlockAckRecord::blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    SequenceNumberCyclic sequenceNumber = header->getSequenceNumber();
    FragmentNumber fragmentNumber = header->getFragmentNumber();
    acknowledgmentState[SequenceControlField(sequenceNumber.get(), fragmentNumber)] = true;
}

bool BlockAckRecord::getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber)
{
    // The status of MPDUs that are considered “old” and prior to the sequence number
    // range for which the receiver maintains status shall be reported as successfully
    // received (i.e., the corresponding bit in the bitmap shall be set to 1).
    if (containsKey(acknowledgmentState, SequenceControlField(sequenceNumber.get(), fragmentNumber))) {
        return true;
    }
    else if (acknowledgmentState.size() == 0) {
        return true; // TODO old?
    }
    else {
        auto earliest = acknowledgmentState.begin();
        return SequenceNumberCyclic(earliest->first.getSequenceNumber()) > sequenceNumber; // old = true
    }
}

bool BlockAckRecord::getCompressedAckState(SequenceNumberCyclic sequenceNumber)
{
    // IEEE Std 802.11-2024, 10.25.6.1: bits preceding the maintained
    // receive-window range are one; missing MPDUs within the range are zero.
    return containsKey(acknowledgmentState, SequenceControlField(sequenceNumber.get(), 0)) || sequenceNumber < startingSequenceNumber;
}

void BlockAckRecord::removeAckStates(SequenceNumberCyclic sequenceNumber)
{
    auto it = acknowledgmentState.begin();
    while (it != acknowledgmentState.end()) {
        if (SequenceNumberCyclic(it->first.getSequenceNumber()) < sequenceNumber)
            it = acknowledgmentState.erase(it);
        else
            it++;
    }
    if (startingSequenceNumber <= sequenceNumber)
        startingSequenceNumber = sequenceNumber + 1;
}

} /* namespace ieee80211 */
} /* namespace inet */
