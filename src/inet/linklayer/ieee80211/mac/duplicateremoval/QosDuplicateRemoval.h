//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSDUPLICATEREMOVAL_H
#define __INET_QOSDUPLICATEREMOVAL_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"

namespace inet {
namespace ieee80211 {

class INET_API QoSDuplicateRemoval : public IDuplicateRemoval
{
  protected:
    typedef std::pair<MacAddress, Tid> Key;
    typedef std::map<Key, SequenceControlField> Key2SeqValMap;
    typedef std::map<MacAddress, SequenceControlField> Mac2SeqValMap;
    Key2SeqValMap lastSeenSeqNumCache; // cache of last seen sequence numbers per TA
    Mac2SeqValMap lastSeenSharedSeqNumCache;
    Mac2SeqValMap lastSeenTimePriorityManagementSeqNumCache;

  public:
    virtual bool isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
};

} // namespace ieee80211
} // namespace inet

#endif

