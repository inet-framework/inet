//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga, Benjamin Seregi
//

#include "DuplicateDetectors.h"
#include "Ieee80211Frame_m.h"
#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

void LegacyDuplicateDetector::assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    lastSeqNum = (lastSeqNum + 1) % 4096;
    frame->setSequenceNumber(lastSeqNum);
}

bool LegacyDuplicateDetector::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    const MACAddress& address = frame->getTransmitterAddress();
    SeqVal seqVal;
    seqVal.seqNum = frame->getSequenceNumber();
    seqVal.fragNum = frame->getFragmentNumber();
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache[address] = seqVal;
    else if (it->second.seqNum == seqVal.seqNum && it->second.fragNum == seqVal.fragNum && frame->getRetry())
        return true;
    else
        it->second = seqVal;
    return false;
}

//---

void NonQoSDuplicateDetector::assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    lastSeqNum = (lastSeqNum + 1) % 4096;
    const MACAddress& address = frame->getReceiverAddress();
    auto it = lastSentSeqNums.find(address);
    if (it == lastSentSeqNums.end())
        lastSentSeqNums[address] = lastSeqNum;
    else {
        if (it->second == lastSeqNum)
            lastSeqNum = (lastSeqNum + 1) % 4096; // make it different from the last sequence number sent to that RA (spec: "add 2")
        it->second = lastSeqNum;
    }
    frame->setSequenceNumber(lastSeqNum);
}

//---

QoSDuplicateDetector::CacheType QoSDuplicateDetector::getCacheType(Ieee80211DataOrMgmtFrame *frame, bool incoming)
{
    bool isTimePriorityFrame = false; // TODO
    const MACAddress& address = incoming ? frame->getTransmitterAddress() : frame->getReceiverAddress();
    if (isTimePriorityFrame)
        return TIME_PRIORITY;
    else if (frame->getType() != ST_DATA_WITH_QOS || address.isMulticast())
        return SHARED;
    else
        return DATA;
}

void QoSDuplicateDetector::assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame)
{
    CacheType type = getCacheType(frame, false);
    int seqNum;
    MACAddress address = frame->getReceiverAddress();
    if (type == TIME_PRIORITY)
    {
        // Error in spec?
        // "QoS STA may use values from additional modulo-4096 counters per <Address 1, TID> for sequence numbers assigned to
        // time priority management frames." 9.3.2.10 Duplicate detection and recovery, But management frames don't have QoS Control field.
        auto it = lastSentTimePrioritySeqNums.find(address);
        if (it == lastSentTimePrioritySeqNums.end())
            lastSentTimePrioritySeqNums[address] = seqNum = 0;
        else
            it->second = seqNum = (it->second + 1) % 4096;
    }
    if (type == SHARED)
    {
        auto it = lastSentSharedSeqNums.find(address);
        if (it == lastSentSharedSeqNums.end())
            lastSentSharedSeqNums[address] = seqNum = lastSentSharedCounterSeqNum;
        else {
            if (it->second == lastSentSharedCounterSeqNum)
                lastSentSharedCounterSeqNum = (lastSentSharedCounterSeqNum + 1) % 4096; // make it different from the last sequence number sent to that RA (spec: "add 2")
            it->second = seqNum = lastSentSharedCounterSeqNum;
        }
    }
    else if (type == DATA)
    {
        Ieee80211DataFrame *qosDataFrame = check_and_cast<Ieee80211DataFrame *>(frame);
        Key key(frame->getReceiverAddress(), qosDataFrame->getTid());
        auto it = lastSentSeqNums.find(key);
        if (it == lastSentSeqNums.end())
            lastSentSeqNums[key] = seqNum = 0;
        else
            it->second = seqNum = (it->second + 1) % 4096;
    }
    else
        ASSERT(false);

    frame->setSequenceNumber(seqNum);
}

bool QoSDuplicateDetector::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    SeqVal seqVal;
    seqVal.seqNum = frame->getSequenceNumber();
    seqVal.fragNum = frame->getFragmentNumber();
    bool isManagementFrame = dynamic_cast<Ieee80211ManagementFrame *>(frame);
    bool isTimePriorityManagementFrame = isManagementFrame && false; // TODO: hack
    if (isTimePriorityManagementFrame || isManagementFrame)
    {
        MACAddress transmitterAddr = frame->getTransmitterAddress();
        Mac2SeqValMap& cache = isTimePriorityManagementFrame ? lastSeenTimePriorityManagementSeqNumCache : lastSeenSharedSeqNumCache;
        auto it = cache.find(transmitterAddr);
        if (it == cache.end())
            cache[transmitterAddr] = seqVal;
        if (it->second.seqNum == seqVal.seqNum && it->second.fragNum == seqVal.fragNum && frame->getRetry())
            return true;
        else
            it->second = seqVal;
        return false;
    }
    else
    {
        Ieee80211DataFrame *qosDataFrame = check_and_cast<Ieee80211DataFrame *>(frame);
        Key key(frame->getTransmitterAddress(), qosDataFrame->getTid());
        auto it = lastSeenSeqNumCache.find(key);
        if (it == lastSeenSeqNumCache.end())
            lastSeenSeqNumCache[key] = seqVal;
        if (it->second.seqNum == seqVal.seqNum && it->second.fragNum == seqVal.fragNum && frame->getRetry())
            return true;
        else
            it->second = seqVal;
        return false;
    }
}

} // namespace ieee80211
} // namespace inet
