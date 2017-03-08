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

#include "inet/common/stlutils.h"
#include "QosDuplicateRemoval.h"

namespace inet {
namespace ieee80211 {

bool QoSDuplicateRemoval::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    SequenceControlField seqVal(frame);
    bool isManagementFrame = dynamic_cast<Ieee80211ManagementFrame *>(frame);
    bool isTimePriorityManagementFrame = isManagementFrame && false; // TODO: hack
    if (isTimePriorityManagementFrame || isManagementFrame)
    {
        MACAddress transmitterAddr = frame->getTransmitterAddress();
        Mac2SeqValMap& cache = isTimePriorityManagementFrame ? lastSeenTimePriorityManagementSeqNumCache : lastSeenSharedSeqNumCache;
        auto it = cache.find(transmitterAddr);
        if (it == cache.end()) {
            cache.insert(std::pair<MACAddress, SequenceControlField>(transmitterAddr, seqVal));
            return false;
        }
        else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && frame->getRetry())
            return true;
        else {
            it->second = seqVal;
            return false;
        }
    }
    else
    {
        Ieee80211DataFrame *qosDataFrame = check_and_cast<Ieee80211DataFrame *>(frame);
        Key key(frame->getTransmitterAddress(), qosDataFrame->getTid());
        auto it = lastSeenSeqNumCache.find(key);
        if (it == lastSeenSeqNumCache.end()) {
            lastSeenSeqNumCache.insert(std::pair<Key, SequenceControlField>(key, seqVal));
            return false;
        }
        else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && frame->getRetry())
            return true;
        else {
            it->second = seqVal;
            return false;
        }
    }
}

} // namespace ieee80211
} // namespace inet

