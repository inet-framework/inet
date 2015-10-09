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
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

void LegacyDuplicateDetector::processOutgoingPacket(Ieee80211DataOrMgmtFrame *frame)
{
    lastSeqNum = (lastSeqNum + 1) % 4096;
    frame->setSequenceNumber(lastSeqNum);
}

bool LegacyDuplicateDetector::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    const MACAddress& address = frame->getTransmitterAddress();
    int16_t seqNum = frame->getSequenceNumber();
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache[address] = seqNum;
    else if (it->second == seqNum)
        return true;
    else
        it->second = seqNum;
    return false;
}

//---

void NonQoSDuplicateDetector::processOutgoingPacket(Ieee80211DataOrMgmtFrame *frame)
{
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

QoSDuplicateDetector::Key QoSDuplicateDetector::getKey(Ieee80211DataOrMgmtFrame *frame, bool incoming)
{
    static Key multicastKey(MACAddress::UNSPECIFIED_ADDRESS, 0);
    Ieee80211DataFrame *dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame);
    if (!dataFrame)
        return multicastKey;
    if (frame->getType() != ST_DATA_WITH_QOS)  // has no QoS header
        return multicastKey;
    const MACAddress& address = incoming ? frame->getTransmitterAddress() : frame->getReceiverAddress();
    if (address.isMulticast())
        return multicastKey;
    return Key(address, dataFrame->getTid());
}

void QoSDuplicateDetector::processOutgoingPacket(Ieee80211DataOrMgmtFrame *frame)
{
    Key key = getKey(frame, false);
    int seqNum;
    auto it = lastSentSeqNums.find(key);
    if (it == lastSentSeqNums.end())
        lastSentSeqNums[key] = seqNum = 0;
    else
        it->second = seqNum = (it->second + 1) % 4096;
    frame->setSequenceNumber(seqNum);
}

bool QoSDuplicateDetector::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    Key key = getKey(frame, true);
    int seqNum = frame->getSequenceNumber();
    auto it = lastSeenSeqNumCache.find(key);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache[key] = seqNum;
    else if (it->second == seqNum)
        return true;
    else
        it->second = seqNum;
    return false;
}


} // namespace ieee80211
} // namespace inet
