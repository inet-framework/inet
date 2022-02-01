//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/duplicateremoval/QosDuplicateRemoval.h"

#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

bool QoSDuplicateRemoval::isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    SequenceControlField seqVal(header->getSequenceNumber().get(), header->getFragmentNumber());
    bool isManagementFrame = dynamicPtrCast<const Ieee80211MgmtHeader>(header) != nullptr;
    bool isTimePriorityManagementFrame = isManagementFrame && false; // TODO hack
    if (isTimePriorityManagementFrame || isManagementFrame) {
        MacAddress transmitterAddr = header->getTransmitterAddress();
        Mac2SeqValMap& cache = isTimePriorityManagementFrame ? lastSeenTimePriorityManagementSeqNumCache : lastSeenSharedSeqNumCache;
        auto it = cache.find(transmitterAddr);
        if (it == cache.end()) {
            cache.insert(std::pair<MacAddress, SequenceControlField>(transmitterAddr, seqVal));
            return false;
        }
        else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && header->getRetry())
            return true;
        else {
            it->second = seqVal;
            return false;
        }
    }
    else {
        const Ptr<const Ieee80211DataHeader>& qosDataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
        Key key(header->getTransmitterAddress(), qosDataHeader->getTid());
        auto it = lastSeenSeqNumCache.find(key);
        if (it == lastSeenSeqNumCache.end()) {
            lastSeenSeqNumCache.insert(std::pair<Key, SequenceControlField>(key, seqVal));
            return false;
        }
        else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && header->getRetry())
            return true;
        else {
            it->second = seqVal;
            return false;
        }
    }
}

} // namespace ieee80211
} // namespace inet

