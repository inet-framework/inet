//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LEGACYSEQUENCENUMBERASSIGNMENT_H
#define __INET_LEGACYSEQUENCENUMBERASSIGNMENT_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API LegacySequenceNumberAssignment : public ISequenceNumberAssignment
{
  protected:
    // Store the predecessor so the first increment emits sequence number 0.
    SequenceNumberCyclic lastSeqNum = SequenceNumberCyclic(4095);

  public:
    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

