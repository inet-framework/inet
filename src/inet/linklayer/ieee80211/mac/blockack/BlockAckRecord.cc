//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

BlockAckRecord::BlockAckRecord(MacAddress originatorAddress, Tid tid) :
    originatorAddress(originatorAddress),
    tid(tid)
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

void BlockAckRecord::removeAckStates(SequenceNumberCyclic sequenceNumber)
{
    auto it = acknowledgmentState.begin();
    while (it != acknowledgmentState.end()) {
        if (SequenceNumberCyclic(it->first.getSequenceNumber()) < sequenceNumber)
            it = acknowledgmentState.erase(it);
        else
            it++;
    }
}

void BlockAckRecord::initializeCompressedWindow(SequenceNumberCyclic start, int size)
{
    ASSERT(start.isSpecified() && size > 0 && size <= 64);
    windowStart = start;
    windowSize = size;
    completeReceipts.clear();
}

void BlockAckRecord::advanceCompressedWindow(SequenceNumberCyclic start)
{
    // IEEE Std 802.11-2024, 10.25.6.3: retain explicit receipt truth at WinStartR - 1.
    if (!(windowStart < start))
        return;
    windowStart = start;
    for (auto it = completeReceipts.begin(); it != completeReceipts.end();) {
        auto distance = (*it - windowStart.get() + 4096) % 4096;
        if (distance >= windowSize && distance != 4095)
            it = completeReceipts.erase(it);
        else
            ++it;
    }
}

void BlockAckRecord::compressedFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    ASSERT(header->getFragmentNumber() == 0 && !header->getMoreFragments());
    auto sequence = header->getSequenceNumber();
    // IEEE Std 802.11-2024, 10.25.6.3: advance before duplicate removal or reorder insertion.
    if (windowStart <= sequence && !(sequence < windowStart + windowSize))
        advanceCompressedWindow(sequence - windowSize + 1);
    if (windowStart <= sequence && sequence < windowStart + windowSize)
        completeReceipts.insert(sequence.get());
}

bool BlockAckRecord::hasCompleteReceipt(SequenceNumberCyclic sequenceNumber) const
{
    return completeReceipts.count(sequenceNumber.get()) != 0;
}

bool BlockAckRecord::getCompressedAckState(SequenceNumberCyclic sequenceNumber) const
{
    return windowStart <= sequenceNumber && sequenceNumber < windowStart + windowSize && hasCompleteReceipt(sequenceNumber);
}

} /* namespace ieee80211 */
} /* namespace inet */

