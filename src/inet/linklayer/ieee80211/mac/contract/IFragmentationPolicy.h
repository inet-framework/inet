//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFRAGMENTATIONPOLICY_H
#define __INET_IFRAGMENTATIONPOLICY_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtHeader;

class INET_API IFragmentationPolicy
{
  public:
    virtual ~IFragmentationPolicy() {}

    virtual std::vector<int> computeFragmentSizes(Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

