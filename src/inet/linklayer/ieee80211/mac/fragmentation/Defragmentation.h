//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DEFRAGMENTATION_H
#define __INET_DEFRAGMENTATION_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/contract/IDefragmentation.h"

namespace inet {
namespace ieee80211 {

class INET_API Defragmentation : public IDefragmentation, public cObject
{
  public:
    virtual Packet *defragmentFrames(std::vector<Packet *> *fragmentFrames) override;
};

} // namespace ieee80211
} // namespace inet

#endif

