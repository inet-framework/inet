//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDUPLICATEREMOVAL_H
#define __INET_IDUPLICATEREMOVAL_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IDuplicateRemoval
{
  public:
    virtual ~IDuplicateRemoval() {}

    virtual bool isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

