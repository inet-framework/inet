//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.h"

#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

bool LegacyDuplicateRemoval::isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    ASSERT(header->getType() != ST_DATA_WITH_QOS);
    const MacAddress& address = header->getTransmitterAddress();
    SequenceControlField seqVal(header->getSequenceNumber().get(), header->getFragmentNumber());
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache.insert(std::pair<MacAddress, SequenceControlField>(address, seqVal));
    else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && header->getRetry())
        return true;
    else
        it->second = seqVal;
    return false;
}

} // namespace ieee80211
} // namespace inet

