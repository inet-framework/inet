//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFRAGMENTATION_H
#define __INET_IFRAGMENTATION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtHeader;

class INET_API IFragmentation
{
  public:
    virtual ~IFragmentation() {}

    virtual std::vector<Packet *> *fragmentFrame(Packet *frame, const std::vector<int>& fragmentSizes) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

