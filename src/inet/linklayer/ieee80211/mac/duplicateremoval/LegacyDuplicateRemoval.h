//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LEGACYDUPLICATEREMOVAL_H
#define __INET_LEGACYDUPLICATEREMOVAL_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"

namespace inet {
namespace ieee80211 {

class INET_API LegacyDuplicateRemoval : public IDuplicateRemoval
{
  protected:
    std::map<MacAddress, SequenceControlField> lastSeenSeqNumCache; // cache of last seen sequence numbers per TA

  public:
    virtual bool isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
};

} // namespace ieee80211
} // namespace inet

#endif

