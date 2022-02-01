//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICMPDUAGGREGATIONPOLICY_H
#define __INET_BASICMPDUAGGREGATIONPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicMpduAggregationPolicy : public IMpduAggregationPolicy, public cSimpleModule
{
  protected:
    virtual void initialize() override;

  public:
    virtual std::vector<Packet *> *computeAggregateFrames(std::vector<Packet *> *frames) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

