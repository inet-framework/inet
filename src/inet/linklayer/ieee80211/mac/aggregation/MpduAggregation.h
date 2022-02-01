//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MPDUAGGREGATION_H
#define __INET_MPDUAGGREGATION_H

#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregation.h"

namespace inet {
namespace ieee80211 {

class INET_API MpduAggregation : public IMpduAggregation, public cObject
{
  public:
    virtual Packet *aggregateFrames(std::vector<Packet *> *frames) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

