//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSSEQUENCENUMBERASSIGNMENT_H
#define __INET_QOSSEQUENCENUMBERASSIGNMENT_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API QoSSequenceNumberAssignment : public ISequenceNumberAssignment
{
  protected:
    enum CacheType {
        SHARED,
        TIME_PRIORITY,
        DATA
    };
    typedef std::pair<MacAddress, Tid> Key;

    std::map<Key, SequenceNumberCyclic> lastSentSeqNums; // last sent sequence numbers per RA
    std::map<MacAddress, SequenceNumberCyclic> lastSentTimePrioritySeqNums; // last sent sequence numbers per RA
    std::map<MacAddress, SequenceNumberCyclic> lastSentSharedSeqNums; // last sent sequence numbers per RA
    SequenceNumberCyclic lastSentSharedCounterSeqNum = SequenceNumberCyclic(0);

  protected:
    virtual CacheType getCacheType(const Ptr<const Ieee80211DataOrMgmtHeader>& header, bool incoming);

  public:
    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

