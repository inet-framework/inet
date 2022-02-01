//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISEQUENCENUMBERASSIGNMENT_H
#define __INET_ISEQUENCENUMBERASSIGNMENT_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API ISequenceNumberAssignment
{
  public:
    virtual ~ISequenceNumberAssignment() {}

    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

