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

void BlockAckRecord::dataFrameReceived(const Ptr<const Ieee80211DataHeader>& header, int windowSize)
{
    SequenceNumberCyclic sequenceNumber = header->getSequenceNumber();
    FragmentNumber fragmentNumber = header->getFragmentNumber();
    // IEEE Std 802.11-2024, 10.25.6.3(b), case 3: an old related MPDU
    // does not change the Block Ack record. The cyclic comparison below already
    // restricts the accepted range to [WinStartR, WinStartR + 2047].
    if (!(startingSequenceNumber <= sequenceNumber))
        return;
    // Cases 1 and 2: record an in-window MPDU, or advance WinStartR
    // before recording an MPDU beyond WinEndR (also see 10.25.6.4(c)).
    if (startingSequenceNumber + windowSize <= sequenceNumber && sequenceNumber < startingSequenceNumber + 2048)
        advanceStartingSequenceNumber(sequenceNumber - windowSize + 1);
    acknowledgmentState[SequenceControlField(sequenceNumber.get(), fragmentNumber)] = true;
}

bool BlockAckRecord::getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber)
{
    // The status of MPDUs that are considered “old” and prior to the sequence number
    // range for which the receiver maintains status shall be reported as successfully
    // received (i.e., the corresponding bit in the bitmap shall be set to 1).
    return containsKey(acknowledgmentState, SequenceControlField(sequenceNumber.get(), fragmentNumber)) || sequenceNumber < startingSequenceNumber;
}

bool BlockAckRecord::getCompressedAckState(SequenceNumberCyclic sequenceNumber)
{
    // IEEE Std 802.11-2024, 10.25.6.1: bits preceding the maintained
    // receive-window range are one; missing MPDUs within the range are zero.
    return containsKey(acknowledgmentState, SequenceControlField(sequenceNumber.get(), 0)) || sequenceNumber < startingSequenceNumber;
}

void BlockAckRecord::advanceStartingSequenceNumber(SequenceNumberCyclic newStartingSequenceNumber)
{
    // IEEE Std 802.11-2024, 10.25.6.3 and 10.25.6.4: advance WinStartR
    // for a newer related MPDU or BAR SSN, using the 12-bit sequence space.
    if (!(startingSequenceNumber < newStartingSequenceNumber))
        return;
    auto it = acknowledgmentState.begin();
    while (it != acknowledgmentState.end()) {
        if (SequenceNumberCyclic(it->first.getSequenceNumber()) < newStartingSequenceNumber)
            it = acknowledgmentState.erase(it);
        else
            it++;
    }
    startingSequenceNumber = newStartingSequenceNumber;
}

} /* namespace ieee80211 */
} /* namespace inet */
