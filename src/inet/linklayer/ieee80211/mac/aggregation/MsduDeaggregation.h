//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MSDUDEAGGREGATION_H
#define __INET_MSDUDEAGGREGATION_H

#include "inet/linklayer/ieee80211/mac/contract/IMsduDeaggregation.h"

namespace inet {
namespace ieee80211 {

class INET_API MsduDeaggregation : public IMsduDeaggregation, public cObject
{
  protected:
    virtual void setExplodedFrameAddress(const Ptr<Ieee80211DataHeader>& header, const Ptr<const Ieee80211MsduSubframeHeader>& subframe, const Ptr<const Ieee80211DataHeader>& aMsduHeader);

  public:
    virtual std::vector<Packet *> *deaggregateFrame(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

