//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/QoSSequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

QoSSequenceNumberAssignment::CacheType QoSSequenceNumberAssignment::getCacheType(const Ptr<const Ieee80211DataOrMgmtHeader>& header, bool incoming)
{
    bool isTimePriorityFrame = false; // TODO
    const MacAddress& address = incoming ? header->getTransmitterAddress() : header->getReceiverAddress();
    if (isTimePriorityFrame)
        return TIME_PRIORITY;
    else if (header->getType() != ST_DATA_WITH_QOS || address.isMulticast())
        return SHARED;
    else
        return DATA;
}

void QoSSequenceNumberAssignment::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    CacheType type = getCacheType(header, false);
    SequenceNumberCyclic seqNum;
    MacAddress address = header->getReceiverAddress();
    if (type == TIME_PRIORITY)
    {
        // Error in spec?
        // "QoS STA may use values from additional modulo-4096 counters per <Address 1, TID> for sequence numbers assigned to
        // time priority management frames." 9.3.2.10 Duplicate detection and recovery, But management frames don't have QoS Control field.
        auto it = lastSentTimePrioritySeqNums.find(address);
        if (it == lastSentTimePrioritySeqNums.end())
            lastSentTimePrioritySeqNums[address] = seqNum = SequenceNumberCyclic(0);
        else
            it->second = seqNum = it->second + 1;
    }
    if (type == SHARED)
    {
        auto it = lastSentSharedSeqNums.find(address);
        if (it == lastSentSharedSeqNums.end())
            lastSentSharedSeqNums[address] = seqNum = lastSentSharedCounterSeqNum;
        else {
            if (it->second == lastSentSharedCounterSeqNum)
                lastSentSharedCounterSeqNum = lastSentSharedCounterSeqNum + 1; // make it different from the last sequence number sent to that RA (spec: "add 2")
            it->second = seqNum = lastSentSharedCounterSeqNum;
        }
    }
    else if (type == DATA)
    {
        const Ptr<const Ieee80211DataHeader>& qosDataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
        Key key(header->getReceiverAddress(), qosDataHeader->getTid());
        auto it = lastSentSeqNums.find(key);
        if (it == lastSentSeqNums.end())
            lastSentSeqNums[key] = seqNum = SequenceNumberCyclic(0);
        else
            it->second = seqNum = it->second + 1;
    }
    else
        ASSERT(false);

    header->setSequenceNumber(seqNum);
}

} /* namespace ieee80211 */
} /* namespace inet */
