//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MPDUDEAGGREGATION_H
#define __INET_MPDUDEAGGREGATION_H

#include "inet/linklayer/ieee80211/mac/contract/IMpduDeaggregation.h"

namespace inet {
namespace ieee80211 {

class INET_API MpduDeaggregation : public IMpduDeaggregation, public cObject
{
  public:
    virtual std::vector<Packet *> *deaggregateFrame(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

