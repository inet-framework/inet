//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RTSPOLICY_H
#define __INET_RTSPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RtsPolicy : public ModeSetListener, public IRtsPolicy
{
  protected:
    int rtsThreshold = -1;
    simtime_t ctsTimeout = -1;
    IRateSelection *rateSelection = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual bool isRtsNeeded(Packet *packet, const Ptr<const Ieee80211MacHeader>& protectedHeader) const override;
    virtual simtime_t getCtsTimeout(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
    virtual int getRtsThreshold() const override { return rtsThreshold; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

