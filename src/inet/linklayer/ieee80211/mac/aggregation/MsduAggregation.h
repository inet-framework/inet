//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MSDUAGGREGATION_H
#define __INET_MSDUAGGREGATION_H

#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregation.h"

namespace inet {
namespace ieee80211 {

class INET_API MsduAggregation : public IMsduAggregation, public cObject
{
  protected:
    virtual void setSubframeAddress(const Ptr<Ieee80211MsduSubframeHeader>& subframe, const Ptr<const Ieee80211DataHeader>& header);

  public:
    virtual Packet *aggregateFrames(std::vector<Packet *> *frames) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

