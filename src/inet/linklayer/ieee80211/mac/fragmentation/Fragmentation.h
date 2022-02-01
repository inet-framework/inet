//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTATION_H
#define __INET_FRAGMENTATION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentation.h"

namespace inet {
namespace ieee80211 {

class INET_API Fragmentation : public IFragmentation, public cObject
{
  public:
    virtual std::vector<Packet *> *fragmentFrame(Packet *frame, const std::vector<int>& fragmentSizes) override;
};

} // namespace ieee80211
} // namespace inet

#endif

