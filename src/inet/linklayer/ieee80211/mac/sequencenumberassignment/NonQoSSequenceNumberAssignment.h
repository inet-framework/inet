//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NONQOSSEQUENCENUMBERASSIGNMENT_H
#define __INET_NONQOSSEQUENCENUMBERASSIGNMENT_H

#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/LegacySequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API NonQoSSequenceNumberAssignment : public LegacySequenceNumberAssignment
{
  protected:
    std::map<MacAddress, SequenceNumberCyclic> lastSentSeqNums; // last sent sequence numbers per RA

  public:
    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

