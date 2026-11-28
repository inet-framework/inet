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
  protected:
    Packet *buildAggregate(const std::vector<Packet *>& frames, B minimumStartDistance) const;

  public:
    virtual Packet *aggregateFrames(std::vector<Packet *> *frames) override;
    virtual B computeAggregateLength(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
            int minimumMpduStartSpacing) const override;
    virtual Packet *aggregateFrames(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
            int minimumMpduStartSpacing, B maxAggregateLength) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

