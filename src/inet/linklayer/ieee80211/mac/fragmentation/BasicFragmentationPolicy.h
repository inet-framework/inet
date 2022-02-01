//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICFRAGMENTATIONPOLICY_H
#define __INET_BASICFRAGMENTATIONPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicFragmentationPolicy : public IFragmentationPolicy, public cSimpleModule
{
  protected:
    int fragmentationThreshold = -1;

  protected:
    virtual void initialize() override;

  public:
    virtual std::vector<int> computeFragmentSizes(Packet *frame) override;
};

} // namespace ieee80211
} // namespace inet

#endif

