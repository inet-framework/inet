//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BASICMSDUAGGREGATIONPOLICY_H
#define __INET_BASICMSDUAGGREGATIONPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicMsduAggregationPolicy : public IMsduAggregationPolicy, public cSimpleModule
{
  protected:
    bool qOsCheck = false;
    int subframeNumThreshold = -1;
    int aggregationLengthThreshold = -1;
    b maxAMsduSize = b(-1);

  protected:
    virtual void initialize() override;
    virtual bool isAggregationPossible(int numOfFramesToAggragate, int aMsduLength);
    virtual bool isEligible(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, const Ptr<const Ieee80211MacTrailer>& trailer, const Ptr<const Ieee80211DataHeader>& testHeader, b aMsduLength);

  public:
    virtual std::vector<Packet *> *computeAggregateFrames(queueing::IPacketQueue *queue) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

