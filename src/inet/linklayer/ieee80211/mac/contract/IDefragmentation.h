//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEFRAGMENTATION_H
#define __INET_IDEFRAGMENTATION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtHeader;

class INET_API IDefragmentation
{
  public:
    virtual ~IDefragmentation() {}

    virtual Packet *defragmentFrames(std::vector<Packet *> *fragmentFrames) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

